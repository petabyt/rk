// Generic Interrupt Controller
// GIC500 / GICv3
#include "io.h"
#include "os.h"
#include "pins.h"

/*
Just some vocab:
Locality-specific Peripheral Interrupts (LPIs).
Private Peripheral Interrupts (PPIs).
Software Generated Interrupts (SGIs).
Shared Peripheral Interrupts (SPIs).
*/

// GICD Spec chap 12 page 529
// Holds routing and prio info
struct GICDistributor {
	uint32_t ctlr;
	uint32_t typer;
	uint32_t iidr;
	char res0[0x34];
	uint32_t setspi_nsr;
	uint32_t res1;
	uint32_t clrspi_nsr;	
	uint32_t res2;
	uint32_t setspi_sr;
	uint32_t res3;
	uint32_t clrspi_sr;
	char res4[0x24];
	uint32_t igroup_regs[31]; // int group
	uint32_t res5[1];
	uint32_t isenable_regs[31]; // Int set-enable
	uint32_t res6[1];
	uint32_t icenable_regs[31]; // Int clear-enable
	uint32_t res7[1];
	uint32_t ispend_regs[31]; // Int set-pending
	uint32_t res8[1];
	uint32_t icpend_regs[31]; // Int clear-pending
	uint32_t res9[1];
	uint32_t isactive_regs[31]; // int set-active
	uint32_t res10[1];
	uint32_t icactive_regs[31]; // int-clear-active
	uint32_t res11[1];
	uint32_t ipriority_regs[254]; // int priority
	uint32_t res12[1];
	uint32_t itargets_regs[254]; // int target
	uint32_t res13[1];
	uint32_t icfg_regs[64]; // int configuration
	char res15[0x200];
	uint32_t sgi;
};

// GICC
struct GICCPUInterface {
	uint32_t ctlr; // int ctl reg
	uint32_t pmr; // int priority mask reg

	uint32_t bpr; // binary point reg
	uint32_t iar; // int ack reg
	uint32_t eoi; // end of int reg
	uint32_t rpr; // running priority reg
	uint32_t hppir; // Highest Priority Pending Interrupt Register

	uint32_t abpr; // Aliased Binary Point Register
	uint32_t aiar; // Aliased Interrupt Acknowledge Register
	uint32_t aeoi; // Aliased End of Interrupt Register
	uint32_t ahppir; // Aliased Highest Priority Pending Interrupt Register
	int res0[41];
	uint32_t apr0;
	int res1[3];
	uint32_t nsapr0;
	int res2[6];
	uint32_t iid;
	int res3[960];
	uint32_t dir;
};

// Config settings for PPIs and SGIs
struct GICRedist {
	uint32_t ctlr;
	uint32_t iidr;
	uint32_t typer;
	uint32_t statusr; // (typer is 64 bit)
	uint32_t res0;
	uint32_t waker;
	uint32_t mpamidr;
	uint32_t partidr;
	uint8_t res1[20];
	uint32_t setlpir;
	uint32_t clrlpir;
	uint32_t propbaser;
	uint8_t res2[24];
};

void gicd_wait_for_write() {
	volatile struct GICDistributor *gicd = (volatile struct GICDistributor *)GICD_BASE;
	while ((gicd->ctlr & (1 << 31)) != 0);
}

void print_iidr(uint32_t iidr) {
	debug("Implementer: ", iidr & 0b11111111111);
	debug("Revision: ", (iidr >> 12) & 0b1111);
	debug("Variant: ", (iidr >> 16) & 0b1111);
	debug("Productid: ", (iidr >> 24) & 0b11111111);
}

void gic_info() {
	volatile struct GICDistributor *gicd = (volatile struct GICDistributor *)GICD_BASE;

	uint32_t typer = gicd->typer;
	debug("SecurityExtn: ", (typer >> 10) & 1);
	debug("CPUNumber: ", (typer >> 5) & 0b111);
	debug("EXN impl: ", typer & (1 << 8));
	debug("ITLinesNumber: ", typer & 0b1111);	

	print_iidr(gicd->iidr);
}

void startup_redist() {
	volatile struct GICRedist *gicr = (volatile struct GICRedist *)GICR_BASE;
	gicr->ctlr |= (1 << 0); // LPI support is enabled.
	debug("gicr->ctlr: ", gicr->ctlr);

	print_iidr(gicr->iidr);

	gicr->waker &= ~(1 << 1);

	puts("Waiting for ChildrenAsleep bit to clear");
	while (gicr->waker & 0b100);
	puts("done");
}

int gic_get_int() {
	volatile struct GICCPUInterface *gicc = (volatile struct GICCPUInterface *)GICC_BASE;
	return gicc->iar;
}

void gic_init() {
	volatile struct GICDistributor *gicd = (volatile struct GICDistributor *)GICD_BASE;

	gic_info();

	// Enable affinity routing
	gicd->ctlr = (1 << 4) | (1 << 5); // ARE_NS | ARE_S
	gicd_wait_for_write();

	gicd->ctlr |= (1 << 4) | (1 << 5) |
		0b111; // EnableGrp | EnableGrp1N | EnableGrp1S
	gicd_wait_for_write();

	startup_redist();

	for (int i = 0; i < 16; i++ )
		gicd->igroup_regs[i] = 0xffffffff;

	for (int i = 8; i < 128; i++ )
		gicd->ipriority_regs[i] = 0x80808080; // highest int prio

	for (int i = 2; i < 32; i++ )
		gicd->icfg_regs[i] = 0;

	for (int i = 8; i < 128; i++ )
		gicd->itargets_regs[i] = 0x01010101;

	for (int i = 1; i < 16; i++ )
		gicd->icenable_regs[i] = 0xffffffff;

	for (int i = 0; i < 16; i++ )
		gicd->icpend_regs[i] = 0xffffffff;

	gicd->icenable_regs[0] = 0xffff0000;
	gicd->isenable_regs[0] = 0x0000ffff;

	for (int i = 0; i < 8; i++) {
		gicd->ipriority_regs[i] = 0xa0a0a0a0;
	}

	volatile struct GICCPUInterface *gicc = (volatile struct GICCPUInterface *)GICC_BASE;
	gicc->pmr = 0xf0;

	puts("Interrupt controller enabled");
}

void gic_enable_irq(int n) {
	volatile struct GICDistributor *gicd = (volatile struct GICDistributor *)GICD_BASE;
	gicd->isenable_regs[n / 32] = 1 << (n & 0x1f);
}

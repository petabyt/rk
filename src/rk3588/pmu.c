#include <main.h>
#include "rk3588.h"

#define PD_GPU 0x0
#define PD_NPU 0x1
#define PD_VCODEC 0x2
#define PD_NPUTOP 0x3
#define PD_NPU1 0x4
#define PD_NPU2 0x5
#define PD_VENC0 0x6
#define PD_VENC1 0x7
#define PD_RKVDEC0 0x8
#define PD_RKVDEC1 0x9
#define PD_VDPU 0x0a
#define PD_RGA30 0x0b
#define PD_AV1 0x0c
#define PD_VI 0x0d
#define PD_FEC 0x0e
#define PD_ISP1 0x0f
#define PD_RGA31 0x10
#define PD_VOP 0x11
#define PD_VO0 0x12
#define PD_VO1 0x13
#define PD_AUDIO 0x14
#define PD_PHP 0x15
#define PD_GMAC 0x16
#define PD_PCIE 0x17
#define PD_NVM 0x18
#define PD_NVM0 0x19
#define PD_SDIO 0x1a
#define PD_USB 0x1b
#define PD_SECURE 0x1c
#define PD_SDMMC 0x1d
#define PD_CRYPTO 0x1e
#define PD_CENTER 0x1f
#define PD_DDR01 0x20
#define PD_DDR23 0x21

struct __attribute__((aligned(16))) SetupConfig {
	int gate_sftcon_reg;
	int gate_sftcon_bit;
	int gate_sts_reg;
	int gate_sts_bit;
	int has_biu;
	int biu_idle_sftcon_reg;
	int biu_idle_sftcon_bit;
	int biu_idle_ack_sts_reg;
	int biu_idle_ack_sts_bit;
	int bisr_sts_bit;
	int mem_pwr_gate_sftcon_reg;
	int mem_pwr_gate_sftcon_bit;
	int pwr_mem_sts_reg;
	int pwr_mem_sts_bit;
};

struct __attribute__((packed)) Pmu {
	uint8_t pad1[0x4000];
	uint32_t version;
	uint8_t pad2[0x4108];
	uint32_t biu_idle_sftcon[3];
	uint32_t biu_idle_ack_sts[2];
	uint32_t biu_idle_sts[2];
	uint32_t biu_auto_con[3];
	uint8_t pad3[0xc];
	uint32_t pwr_gate_con[3];
	uint32_t pwr_gate_sftcon[3];
	uint8_t pad4[0x28];
	uint32_t pwr_gate_sts[2];
	uint8_t pad5[0x18];
	uint32_t mem_pwr_gate_sftcon[3];
	uint8_t pad6[4];
	uint32_t submem_pwr_gate_sftcon[3];
	uint8_t pad7[0x34];
	uint32_t pwr_chain1_sts[2];
	uint32_t pwr_mem_sts[2];
	uint32_t bisr_con[0xe];
	uint8_t pad8[0x48];
	uint32_t bisr_sts[0x5];
};

_Static_assert(__builtin_offsetof(struct Pmu, biu_idle_sftcon) == 0x810c, "Failed offset check");
_Static_assert(__builtin_offsetof(struct Pmu, biu_idle_sts) == 0x8120, "Failed offset check");
_Static_assert(__builtin_offsetof(struct Pmu, pwr_gate_sftcon) == 0x814c, "Failed offset check");
_Static_assert(__builtin_offsetof(struct Pmu, submem_pwr_gate_sftcon) == 0x81b0, "Failed offset check");
_Static_assert(__builtin_offsetof(struct Pmu, pwr_chain1_sts) == 0x81f0, "Failed offset check");
_Static_assert(__builtin_offsetof(struct Pmu, bisr_con) == 0x8200, "Failed offset check");
_Static_assert(__builtin_offsetof(struct Pmu, bisr_sts) == 0x8280, "Failed offset check");

static struct SetupConfig get_pd_config(int pd) {
	switch (pd) {
	case PD_VO0:
		return (struct SetupConfig){
			.gate_sftcon_reg = 1,
			.gate_sftcon_bit = 2,
			.gate_sts_reg = 0,
			.gate_sts_bit = 0x12,
			.has_biu = 1,
			.biu_idle_sftcon_reg = 0,
			.biu_idle_sftcon_bit = 0xf,
			.biu_idle_ack_sts_reg = 0,
			.biu_idle_ack_sts_bit = 0xf,
			.bisr_sts_bit = 0x11,
			.pwr_mem_sts_reg = 0x0,
			.pwr_mem_sts_bit = 0x1a,
			.mem_pwr_gate_sftcon_reg = 0x1,
			.mem_pwr_gate_sftcon_bit = 0x2,
		};
	case PD_VO1:
		return (struct SetupConfig){
			.gate_sftcon_reg = 1,
			.gate_sftcon_bit = 3,
			.gate_sts_reg = 0,
			.gate_sts_bit = 0x13,
			.has_biu = 1,
			.biu_idle_sftcon_reg = 1,
			.biu_idle_sftcon_bit = 0,
			.biu_idle_ack_sts_reg = 0,
			.biu_idle_ack_sts_bit = 0x10,
			.bisr_sts_bit = 0x12,
			.pwr_mem_sts_reg = 0,
			.pwr_mem_sts_bit = 0x1b,
			.mem_pwr_gate_sftcon_reg = 1,
			.mem_pwr_gate_sftcon_bit = 3,
		};
	case PD_SECURE:
		return (struct SetupConfig){
			.gate_sftcon_reg = 0x1,
			.gate_sftcon_bit = 0xc,
			.gate_sts_reg = 0x0,
			.gate_sts_bit = 0x1c,
			.has_biu = 1,
			.biu_idle_sftcon_reg = 1,
			.biu_idle_sftcon_bit = 7,
			.biu_idle_ack_sts_reg = 0,
			.biu_idle_ack_sts_bit = 0x17,
			.bisr_sts_bit = -1, // no repair
			.pwr_mem_sts_reg = -1,
			.pwr_mem_sts_bit = -1,
			.mem_pwr_gate_sftcon_reg = -1,
			.mem_pwr_gate_sftcon_bit = -1,
		};
	case PD_PHP:
		return (struct SetupConfig){
			.gate_sftcon_reg = 1,
			.gate_sftcon_bit = 5,
			.gate_sts_reg = 0,
			.gate_sts_bit = 0x15,
			.has_biu = 0,
			.pwr_mem_sts_reg = 0,
			.pwr_mem_sts_bit = 0x1d,
			.mem_pwr_gate_sftcon_reg = 1,
			.mem_pwr_gate_sftcon_bit = 5,
		};
	case PD_VOP:
		return (struct SetupConfig){
			.gate_sftcon_reg = 1,
			.gate_sftcon_bit = 1,
			.gate_sts_reg = 0,
			.gate_sts_bit = 0x11,
			.has_biu = 1,
			.biu_idle_sftcon_reg = 0,
			.biu_idle_sftcon_bit = 0xd,
			.biu_idle_ack_sts_reg = 0,
			.biu_idle_ack_sts_bit = 0xd,
			.bisr_sts_bit = 0x10,
			.pwr_mem_sts_reg = 0,
			.pwr_mem_sts_bit = 0x19,
			.mem_pwr_gate_sftcon_reg = 1,
			.mem_pwr_gate_sftcon_bit = 1,
		};
	default: return (struct SetupConfig){};
	}
}

// TODO: Unused
// BUI = Bus interface unit
int enable_biu(int pd, struct SetupConfig *cfg) {
	volatile struct Pmu *pmu = (volatile struct Pmu *)PMU;
	// Enable sending idle request to PD BIU by software
	rk_clr_set_bits(&pmu->biu_idle_sftcon[cfg->biu_idle_sftcon_reg], cfg->biu_idle_sftcon_bit, cfg->biu_idle_sftcon_bit, 1);
	int timeout = 0x100;
	while (timeout--) {
		if ((pmu->biu_idle_ack_sts[cfg->biu_idle_ack_sts_reg] & (1 << cfg->biu_idle_ack_sts_bit))) break;
		udelay(1);
	}
	if (timeout == 0) {
		puts("Timeout ACK");
		return -1;
	}
	if (cfg->bisr_sts_bit == -1) {
		if (pmu->bisr_sts[4] & (1 << cfg->bisr_sts_bit)) {
			puts("Power down (or repair) not complete");
			return -1;
		}
	}
	return 0;
}

int power_up_memory(int pd, struct SetupConfig *cfg) {
	volatile struct Pmu *pmu = (volatile struct Pmu *)PMU;
	rk_clr_set_bits(&pmu->mem_pwr_gate_sftcon[cfg->mem_pwr_gate_sftcon_reg], cfg->mem_pwr_gate_sftcon_bit, cfg->mem_pwr_gate_sftcon_bit, 1);
	udelay(1000); // wtf
	rk_clr_set_bits(&pmu->mem_pwr_gate_sftcon[cfg->mem_pwr_gate_sftcon_reg], cfg->mem_pwr_gate_sftcon_bit, cfg->mem_pwr_gate_sftcon_bit, 0);

	int timeout = 0x1000;
	while (timeout--) {
		if (!(pmu->pwr_mem_sts[cfg->pwr_mem_sts_reg] & (1 << cfg->pwr_mem_sts_bit))) break;
		udelay(1);
	}
	if (timeout == 0) {
		puts("Timeout powering up memory");
		return -1;
	}
	
	return 0;
}

int power_up(int pd) {
	debug("Powering up pd ", pd);
	volatile struct Pmu *pmu = (volatile struct Pmu *)PMU;

	struct SetupConfig cfg = get_pd_config(pd);

	if ((pmu->pwr_gate_sts[cfg.gate_sts_reg] & (1 << cfg.gate_sts_bit)) == 0) {
		// Power domain already on
		return 0;
	}

	// Disable hardware power down
	rk_clr_set_bits(&pmu->pwr_gate_con[cfg.gate_sftcon_reg], cfg.gate_sftcon_bit, cfg.gate_sftcon_bit, 0);
	// Disable software power down
	rk_clr_set_bits(&pmu->pwr_gate_sftcon[cfg.gate_sftcon_reg], cfg.gate_sftcon_bit, cfg.gate_sftcon_bit, 0);

	// Wait until power up
	int timeout = 0x1000;
	while (timeout--) {
		if ((pmu->pwr_gate_sts[cfg.gate_sts_reg] & (1 << cfg.gate_sts_bit)) == 0) break;
		udelay(1);
	}
	if (timeout == 0) {
		puts("PD power up timeout");
		return -1;
	}

	if (cfg.mem_pwr_gate_sftcon_reg != -1) {
		return power_up_memory(pd, &cfg);
	}

	return 0;
}

int rk3588_init_power_domains(void) {
	volatile struct Pmu *pmu = (volatile struct Pmu *)PMU;
	if (pmu->version != 0x3588) {
		return -1;
	}

	// Enable memory repair by hardware
	pmu->bisr_con[1] = 0xffffffff;
	pmu->bisr_con[2] = 0xffffffff;
	pmu->bisr_con[3] = 0xffffffff;

	// Enable automatic clock open/gating
	pmu->biu_auto_con[0] = 0xffffffff;
	pmu->biu_auto_con[1] = 0xffffffff;

	if (power_up(PD_PHP)) return -1;
	if (power_up(PD_SECURE)) return -1;
	if (power_up(PD_VOP)) return -1;
	if (power_up(PD_VO1)) return -1;

	return 0;
}

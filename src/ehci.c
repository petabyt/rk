#include <stdint.h>
#include "os.h"

struct ehci_hcor {
    uint32_t usbcmd;           /* 0x00: USB Command */
    uint32_t usbsts;           /* 0x04: USB Status */
    uint32_t usbintr;          /* 0x08: USB Interrupt Enable */
    uint32_t frindex;          /* 0x0c: USB Frame Index */
    uint32_t ctrldssegment;    /* 0x10: 4G Segment Selector */
    uint32_t periodiclistbase; /* 0x14: Frame List Base Address */
    uint32_t asynclistaddr;    /* 0x18: Next Asynchronous List Address */
    uint32_t reserved[9];
    uint32_t configflag; /* 0x40: Configured Flag Register */
    uint32_t portsc[15]; /* 0x44: Port Status/Control */
};

struct ehci_hccr {
    uint8_t caplength;        /* 0x00: Capability Register Length */
    uint8_t reserved;         /* 0x01: reserved */
    uint16_t hciversion;      /* 0x02: Interface Version Number */
    uint32_t hcsparams;       /* 0x04: Structural Parameters */
    uint32_t hccparams;       /* 0x08: Capability Parameters */
    uint8_t hcspportroute[8]; /* 0x0c: Companion Port Route Description */
};

int setup_ehci(uintptr_t base) {
	volatile struct ehci_hccr *hcor = (volatile struct ehci_hccr *)base;

	debug("caplength: ", hcor->caplength);
	debug("hciversion: ", hcor->hciversion);

	return 0;
}

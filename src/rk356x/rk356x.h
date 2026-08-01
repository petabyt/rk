#ifndef RK356X_H
#define RK356X_H

#include <stdint.h>
#include "edid.h"

#define RK356X_PMUGRF       0xfdc20000UL
#define RK356X_GRF          0xfdc60000UL
#define RK356X_SGRF         0xfdd18000UL
#define RK356X_PMUCRU       0xfdd00000UL
#define RK356X_CRU          0xfdd20000UL
#define RK356X_PMU          0xfdd90000UL
#define RK356X_UART2        0xfe660000UL
#define RK356X_VOP2         0xfe040000UL
#define RK356X_HDMI         0xfe0a0000UL
#define RK356X_OHCI0        0xfd840000UL
#define RK356X_OHCI1        0xfd8c0000UL

#define RK356X_PAYLOAD      0x00a00000UL
#define RK356X_STACK_BOTTOM 0x07ff0000UL
#define RK356X_STACK_TOP    0x08000000UL
#define RK356X_DMA_START    0x08000000UL
#define RK356X_DMA_END      0x08400000UL
#define RK356X_DMA_LIMIT    0x08300000UL
#define RK356X_DTB          0x08300000UL
#define RK356X_SHARED       0x083f0000UL
#define RK356X_FB_START     0x10000000UL
#define RK356X_FB_END       0x12000000UL
#define RK356X_MMIO_START   0xf0000000UL

struct Rk356xGpioPin {
	uint8_t bank;
	uint8_t pin;
	uint8_t active_high;
	uint8_t valid;
};

struct Rk356xBoard {
	const char *name;
	const char *vendor;
	const char *product;
	const char *soc;
	const char *connector_notes;
	struct Rk356xGpioPin leds[2];
	struct Rk356xGpioPin usb_vbus[2];
	uint8_t ohci_mask;
};

struct Rk356xVideo {
	struct VideoMode mode;
	uint32_t stride;
	uint8_t hpd;
	uint8_t edid_valid;
	uint8_t active;
};

extern const struct Rk356xBoard *rk356x_board;
extern struct Rk356xVideo rk356x_video;

void rk356x_enable_uart(void);
void rk356x_setup_security(void);
int rk356x_enable_vo_domain(void);
void rk356x_gpio_output(struct Rk356xGpioPin pin, int asserted);
uint64_t rk356x_detect_dram(void);
void rk356x_set_dtb(const void *data, unsigned int size);
int rk356x_display_init(void);
void rk356x_usb_init(void);
int rk356x_board_entry(const struct Rk356xBoard *board,
		const void *dtb, unsigned int dtb_size);

#endif

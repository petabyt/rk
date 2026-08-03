#include "main.h"
#include "rk356x/rk356x.h"
#include "rk356x/yy3568.dtb.out.h"

static const struct Rk356xBoard board = {
	.name = "Youyeetoo YY3568",
	.vendor = "Youyeetoo",
	.product = "YY3568",
	.soc = "RK3568",
	.connector_notes = "GPIO0_D6 powers both supported USB2 host companions",
	.leds = {
		{ 3, RK_PIN_A4, 1, 1 },
		{ 2, RK_PIN_B2, 1, 1 },
	},
	.usb_vbus = {
		{ 0, RK_PIN_D6, 1, 1 },
		{ 0, 0, 0, 0 },
	},
	.ohci_mask = 3,
};

int c_entry(void) {
	return rk356x_board_entry(&board, dtb_data, sizeof(dtb_data));
}

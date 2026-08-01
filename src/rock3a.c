#include "main.h"
#include "rk356x/rk356x.h"
#include "rk356x/rock3a.dtb.out.h"

static const struct Rk356xBoard board = {
	.name = "Radxa ROCK 3A",
	.vendor = "Radxa",
	.product = "ROCK 3A",
	.soc = "RK3568",
	.connector_notes = "GPIO0_A6 powers the USB hosts; GPIO0_D5 powers the onboard hub",
	.leds = {
		{ 0, RK_PIN_B7, 1, 1 },
		{ 0, 0, 0, 0 },
	},
	.usb_vbus = {
		{ 0, RK_PIN_A6, 1, 1 },
		{ 0, RK_PIN_D5, 1, 1 },
	},
	.ohci_mask = 3,
};

int c_entry(void) {
	return rk356x_board_entry(&board, dtb_data, sizeof(dtb_data));
}

#include "main.h"
#include "rk356x/rk356x.h"
#include "rk356x/roc3566.dtb.out.h"

static const struct Rk356xBoard board = {
	.name = "Firefly ROC-RK3566-PC",
	.vendor = "Firefly",
	.product = "ROC-RK3566-PC",
	.soc = "RK3566",
	.connector_notes = "USB2 host is the USB-A companion path",
	.leds = {
		{ 0, RK_PIN_D3, 1, 1 },
		{ 0, 0, 0, 0 },
	},
	.usb_vbus = {
		{ 0, RK_PIN_C5, 1, 1 },
		{ 0, 0, 0, 0 },
	},
	.ohci_mask = 1,
};

int c_entry(void) {
	return rk356x_board_entry(&board, dtb_data, sizeof(dtb_data));
}

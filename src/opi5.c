#include <string.h>
#include <stdint.h>
#include <rk3588.h>
#include "main.h"

int c_entry() {
	rk3588_setup_mmu();
	puts("Hello, World");
	return 0;
}

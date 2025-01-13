#include <string.h>
#include <stdint.h>
#include "os.h"

void int_handler() {
	
}

int bmp_print_char(char c) {return 0;}

void dump_bootrom() {
	cheap_memdump((void *)0xffff0000, 0x8000);
}

int c_entry() {
	puts("Hello, World");

//	uintptr_t con2 = (0x0fd580000 + 0x0308);
//	debug("Remap: ", ((uint32_t *)con2)[0] );

	//dump_bootrom();

	boot_uboot();

	return 0;
}

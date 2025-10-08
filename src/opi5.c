#include <string.h>
#include <stdint.h>
#include "os.h"

void dump_bootrom(void) {
	cheap_memdump((void *)0xffff0000, 0x8000);
}

int c_entry() {
	puts("Hello, World");
	return 0;
}

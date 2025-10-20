#include "os.h"
#include "firmware.h"

uint64_t process_svc(uint64_t p1, uint64_t p2, uint64_t p3) {
	switch (p1) {
	case PSCI_VERSION:
		return 0;
	case RK_PRINT_CHAR:
		putchar((char)p2);
		return 0;
	}

	return 0; // error
}

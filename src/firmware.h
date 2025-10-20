#include <stdint.h>
#include <stddef.h>

#define PSCI_VERSION 0x84000000
#define PSCI_SYSTEM_OFF 0x84000008
#define PSCI_SYSTEM_RESET 0x84000009

#define RK_PRINT_CHAR         0xf0000000
#define RK_PRINT_STR          0xf0000001
#define RK_POLL_CHAR          0xf0000002
#define RK_GET_CHAR           0xf0000004
#define RK_FRAMEBUFFER_EXISTS 0xf0000005
#define RK_GET_FRAMEBUFFER    0xf0000006
#define RK_GET_MEM_CHUNK      0xf0010000
#define RK_DTB_EXISTS         0xf0010001
#define RK_GET_DTB            0xf0010002
#define RK_ACPI_EXISTS        0xf0010003
#define RK_GET_ACPI           0xf0010004
#define RK_GET_GIC            0xf0010005
#define RK_GET_XHCI_LIST      0xf0010007
#define RK_GET_USB3_LIST      0xf0010008
#define RK_GET_OHCI_LIST      0xf0010009
#define RK_GET_SDHCI_LIST     0xf001000a
#define RK_GET_DWSD_LIST      0xf001000b

struct __attribute__((packed)) Framebuffer {
	uint64_t address;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
};

struct __attribute__((packed)) Memory {
	uint64_t start_addr;
	uint64_t end_addr;
};

struct __attribute__((packed)) MmioDevices {
	uint32_t length;
	struct __attribute__((packed)) MmioDevice {
		uint64_t address;
		uint32_t spi_interrupts[8];
	}devices[];
};

struct __attribute__((packed)) MmioGic {
	uint32_t exists;
	uint64_t distrib_addr;
	uint64_t redist_addr;
	uint64_t cpuinterf_addr;
};

#define PAYLOAD_FLAG_REQUIRES_RELOCATION (1 << 0)
struct __attribute__((packed)) PayloadHeader {
	uint8_t boot_code[0x28];
	uint32_t magic;
	uint32_t version;
	uint32_t flags;
	uint64_t relocation_addr;
	uint32_t device_id;
	uint32_t stack_pointer;
	uint32_t stack_size;
	uint32_t fubs_image_entry;
};

_Static_assert(sizeof(struct PayloadHeader) <= 0x50, "Payload header size check");
_Static_assert(offsetof(struct PayloadHeader, stack_pointer) == 0x40, "check offset");

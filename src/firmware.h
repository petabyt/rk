// Specification of FUEFI (FUto Extensible Firmware Interface)
#pragma once
#include <stdint.h>
#include <stddef.h>

/// For debugging purposes, the payload can be booted in EL3 and this function signature
/// stored in x0 can be used to call into the firmware
typedef uint64_t fu_call_handler(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3);

#define PAYLOAD_FLAG_REQUIRES_RELOCATION (1 << 0)
#define PAYLOAD_FLAG_POSITION_INDEPENDENT (1 << 1)
struct __attribute__((packed)) FuPayloadHeader {
	/// This small section can contain setup code to setup registers and jump to
	/// whatever runs next.
	uint8_t boot_code[0x28];
	/// Expected to be 0x8008135
	uint32_t magic;
	/// Version of FUEFI this binary expects
	uint32_t version;
	/// See PAYLOAD_FLAG_*
	uint32_t flags;
	// Size of this entire image, including header
	uint32_t img_size;
	/// If flags & PAYLOAD_FLAG_REQUIRES_RELOCATION, then the binary is relocated
	/// to this address.
	uint64_t relocation_addr;
	uint32_t res1;
	uint32_t res2;
	uint32_t res3;
	uint32_t res4;
};

_Static_assert(sizeof(struct FuPayloadHeader) == 0x50, "Payload header size check");

// ARM Standard PSCI smc/svc commands
#define PSCI_VERSION 0x84000000
#define PSCI_SYSTEM_OFF 0x84000008
#define PSCI_SYSTEM_RESET 0x84000009

// Prints a single character
#define FU_PRINT_CHAR         0xf0000000
// Prints a C string
#define FU_PRINT_STR          0xf0000001
// Get a character from uart
#define FU_GET_CHAR           0xf0000002
// Check if character is available in uart
#define FU_POLL_CHAR          0xf0000003
// Get the largest memory chunk under an address
#define FU_GET_MEM_CHUNK      0xf0000004
// Returns 1 if DTB is available
#define FU_DTB_EXISTS         0xf0000005
// Returns the address of the DTB
#define FU_GET_DTB            0xf0000006
// Returns 1 if ACPI table is available
#define FU_ACPI_EXISTS        0xf0000007
// Returns the address of the ACPI table
#define FU_GET_ACPI           0xf0000008

// Get a list of framebuffer screens
#define FU_GET_SCREEN_LIST    0xf0010000
// Get info on the GIC interrupt controller
#define FU_GET_GIC            0xf0010001
// Get a list of XHCI controllers
#define FU_GET_XHCI_LIST      0xf0010002
#define FU_GET_USB3_LIST      0xf0010003
#define FU_GET_OHCI_LIST      0xf0010004
// Get a list of SDHCI controllers
#define FU_GET_SDHCI_LIST     0xf0010005
// Get a list of DesignWare SD controllers
#define FU_GET_DWSD_LIST      0xf0010006

#define FU_STORAGE_READ       0xf0020000
#define FU_STORAGE_WRITE      0xf0020001
#define FU_SET_BRIGHTNESS     0xf0020002

struct __attribute__((packed)) FuScreenList {
	uint32_t length;
	uint32_t pad;
	struct __attribute__((packed)) FuScreen {
		uint64_t framebuffer_addr;
		uint32_t width;
		uint32_t height;
		uint32_t stride;
		uint32_t id;
	}screens[];
};

struct __attribute__((packed)) FuFramebuffer {
	uint64_t address;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
};

struct __attribute__((packed)) FuMemory {
	uint64_t start_addr;
	uint64_t end_addr;
};

struct __attribute__((packed)) FuMmioDeviceList {
	uint32_t length;
	uint32_t pad;
	struct __attribute__((packed)) FuMmioDevice {
		uint64_t address;
		uint32_t n_interrupts;
		uint32_t interrupts[0x11];
	}devices[];
};

struct __attribute__((packed)) FuMmioGic {
	uint32_t exists;
	uint32_t pad;
	uint64_t distrib_addr;
	uint64_t redist_addr;
	uint64_t cpuinterf_addr;
};

struct __attribute__((packed)) FuMemoryMap {
	uint32_t length;
	uint32_t pad;
	struct __attribute__((packed)) FuMemoryMapItem {
		uint64_t start_addr;
		uint64_t end_addr;
		uint32_t flags;
		uint32_t pad2;
	}items;
};

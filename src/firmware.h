// Specification of FUEFI (FUto Extensible Firmware Interface)
#pragma once
#include <stdint.h>
#include <stddef.h>

/// For debugging purposes, the payload can be booted in EL3 and this function signature
/// stored in x0 can be used to call into the firmware
typedef uint64_t fu_call_handler(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3);

#define PAYLOAD_FLAG_REQUIRES_RELOCATION (1 << 0)
#define PAYLOAD_FLAG_POSITION_INDEPENDENT (1 << 1)
struct __attribute__((packed)) PayloadHeader {
	/// This small section can contain setup code to setup registers and jump to
	/// whatever runs next.
	uint8_t boot_code[0x28];
	/// Expected to be 0x8008135
	uint32_t magic;
	/// Version of FUEFI this binary expects
	uint32_t version;
	uint32_t flags;
	// Reserved for future use
	uint32_t res0;
	/// If flags & PAYLOAD_FLAG_REQUIRES_RELOCATION, then the binary is relocated
	/// to this address.
	uint64_t relocation_addr;
	uint32_t user1;
	uint32_t user2;
	uint32_t user3;
	uint32_t user4;
};

_Static_assert(sizeof(struct PayloadHeader) == 0x50, "Payload header size check");

// ARM Standard PSCI smc/svc commands
#define PSCI_VERSION 0x84000000
#define PSCI_SYSTEM_OFF 0x84000008
#define PSCI_SYSTEM_RESET 0x84000009

#define FU_PRINT_CHAR         0xf0000000
#define FU_PRINT_STR          0xf0000001
#define FU_POLL_CHAR          0xf0000002
#define FU_GET_CHAR           0xf0000004
#define FU_FRAMEBUFFER_EXISTS 0xf0000005
#define FU_GET_FRAMEBUFFER    0xf0000006
#define FU_GET_MEM_CHUNK      0xf0010000
#define FU_DTB_EXISTS         0xf0010001
#define FU_GET_DTB            0xf0010002
#define FU_ACPI_EXISTS        0xf0010003
#define FU_GET_ACPI           0xf0010004
#define FU_GET_GIC            0xf0010005
#define FU_GET_XHCI_LIST      0xf0010007
#define FU_GET_USB3_LIST      0xf0010008
#define FU_GET_OHCI_LIST      0xf0010009
#define FU_GET_SDHCI_LIST     0xf001000a
#define FU_GET_DWSD_LIST      0xf001000b

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

struct __attribute__((packed)) FuMmioDevices {
	uint32_t length;
	struct __attribute__((packed)) FuMmioDevice {
		uint64_t address;
		uint32_t interrupts[8];
	}devices[];
};

struct __attribute__((packed)) FuMmioGic {
	uint32_t exists;
	uint64_t distrib_addr;
	uint64_t redist_addr;
	uint64_t cpuinterf_addr;
};

// Specification for an extension of PSCI that provides UEFI-like functionality
// Calls into this interface are either done through smc or a function pointer
#pragma once
#include <stdint.h>
#include <stddef.h>

#define FU_ERROR 0xffffffffffffffff

/// A pointer to this function signature is stored in x0 when entering the payload binary.
/// It can be called instead of using smc to trigger the firmware code.
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
#define PSCI_VERSION          0x84000000
#define PSCI_SYSTEM_OFF       0x84000008
#define PSCI_SYSTEM_RESET     0x84000009
#define PSCI_FEATURES         0x8400000a

/// Basic system commands
#define FU_PRINT_CHAR         0xf0000000
#define FU_PRINT_STR          0xf0000001
#define FU_GET_CHAR           0xf0000002
#define FU_POLL_CHAR          0xf0000003
#define FU_GET_MEM_CHUNK      0xf0000004
#define FU_GET_MEM_MAP        0xf0000005

/// 0xf001xxxx: All of these return structures that start with a length/exists field.
/// Returning 4 bytes of 0 can be used to skip all of them.
#define FU_GET_SCREEN_LIST    0xf0010000
#define FU_GET_GIC            0xf0010001
#define FU_GET_XHCI_LIST      0xf0010002
#define FU_GET_USB3_LIST      0xf0010003
#define FU_GET_OHCI_LIST      0xf0010004
#define FU_GET_SDHCI_LIST     0xf0010005
#define FU_GET_DWSD_LIST      0xf0010006

/// Generic commands
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
	}screens[]; // set limit so it can be defined as global
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

// Map to any type
#define FU_MEM_ATTR_UNUSED (1 << 0)
// Map to normal memory
#define FU_MEM_ATTR_RESERVED (1 << 1)
// Map to write through memory
#define FU_MEM_ATTR_FRAMEBUFFER (1 << 2)
// map to ngnrne device memory
#define FU_MEM_ATTR_MMIO (1 << 3)
// Payload is in this region
#define FU_MEM_ATTR_PAYLOAD (1 << 4)
struct __attribute__((packed)) FuMemoryMap {
	uint32_t length;
	uint32_t pad;
	struct __attribute__((packed)) FuMemoryMapItem {
		uint64_t start_addr;
		uint64_t end_addr;
		uint32_t flags;
		uint32_t pad2;
	}items[];
};

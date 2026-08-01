# RK356x common bare-metal firmware

This page describes behavior shared by the normal and demo firmware variants
for the explicitly supported RK356x boards. Consult the selected [board
page](index.md#documentation-map) for its GPIOs, enabled USB controllers, DDR
loader, artifacts, and validation status.

## Boot and execution model

RK356x BootROM is responsible for reading the RKNS image from SD or receiving
it through MaskROM USB. It runs the packaged Rockchip DDR loader, copies this
project's image into RAM, and transfers control at EL3. The firmware does not
need an SD, eMMC, or USB-storage driver for this delivery step.

Every normal RK356x target ends common initialization by calling
`jump_to_payload()`. It expects a FUEFI payload header immediately after the
linked firmware image, relocates the payload to `0x00a00000` when requested,
and enters it at EL2. A normal `<board>.bin` or `<board>.img` is therefore a
developer base, not a standalone program. Without an appended payload it logs
`Bad payload magic` and halts.

A `demo_<board>.bin` or `demo_<board>.img` appends the repository's example
payload. Use the demo for standalone UART, display, input, DTB, memory-map, EL2
handoff, and reset testing. Booting a demo from SD still does not give the
payload runtime access to the card.

The optional BL31/U-Boot variant is a separately linked program and does not
call `jump_to_payload()`. See [RK356x chainloading](chainloading.md).

## Common initialization

The normal firmware initializes the selected board's:

- UART2 M0 at 1,500,000 baud for early diagnostics.
- DRAM accounting and target-specific MMU map.
- VOP2 VP0, an eSmart XRGB8888 plane, DW-HDMI, DDC, and HDMI PHY.
- Board-enabled USB2 PHY and OHCI companions for polled HID input.
- Minimal firmware DTB and FUEFI service table.
- Reset-to-MaskROM path.

It intentionally has no runtime SD, eMMC, PCIe, NVMe, filesystem, networking,
USB hub, EHCI, or xHCI implementation.

## Memory layout

The normal firmware uses these RK356x low-memory reservations:

| Range | Use | Mapping |
| --- | --- | --- |
| `0x001fe000–0x00200000` | validated DDR ATAG area | reserved |
| `0x00a00000` | requested FUEFI payload relocation | cacheable |
| `0x07ff0000–0x08000000` | EL3/EL2 split stack | cacheable |
| `0x08000000–0x08400000` | OHCI DMA, DTB, and FUEFI exchange | non-cacheable |
| `0x10000000–0x12000000` | maximum framebuffer arena | non-cacheable |
| `0xf0000000–0xffffffff` | MMIO | device |

DRAM size comes first from a checksummed `ATAG_DDR_MEM` record, then from
PMUGRF DDR geometry, and finally from a conservative 1 GiB fallback. FUEFI
reports the loaded image, payload, stacks, USB/FUEFI arena, framebuffer, free
RAM, and MMIO as disjoint ranges. `FU_GET_MEM_CHUNK` returns the largest
remaining free range.

The chainloader has a separate linker and address policy. Do not reuse these
normal-firmware reservations as a board's BL31/BL33 policy.

## HDMI and framebuffer

HDMI detection and EDID selection run once during boot. The firmware accepts
progressive RGB 8-bpc timings up to 3840×2160p30 and a 297 MHz pixel/TMDS
clock. It rejects interlaced, YUV-only, 4:2:0-required, and higher-clock modes.

The preferred or native supported timing wins. Otherwise the firmware selects
the supported timing with the greatest pixel area and then refresh rate. With
HPD and unusable EDID it falls back to 1280×720p60. Without HPD it continues
headless and reports an empty screen list. Runtime hotplug and retraining are
out of scope.

`FU_GET_SCREEN_LIST` reports the selected address, dimensions, 64-byte-aligned
stride, and XRGB8888 type. The fatal-screen path and demo renderer use the
selected geometry rather than assuming 1920×1080.

## USB keyboard input

Each OHCI companion enabled by the board descriptor is initialized and polled.
The implementation supports directly attached low/full-speed HID boot
keyboards, including boot-keyboard interfaces in composite devices. It handles
detach and later reattachment without blocking firmware.

US-ANSI key-down events produce printable ASCII plus Enter, Backspace, Tab,
Escape, Shift, and Caps Lock behavior. Held-key duplicates are suppressed.
Hubs, non-boot reports, arrows, function keys, Alt, EHCI, and xHCI are outside
the implemented contract.

`FU_POLL_CHAR` and `FU_GET_CHAR` service OHCI before reading the input ring.
Polling is nonblocking and `FU_GET_CHAR` returns zero when no character is
available.

## Build and validation

Build all normal/demo artifacts without fetching U-Boot:

```sh
make all
make SHELL=/bin/bash check -j"$(nproc)"
```

Typical RAM-only and SD demo targets are listed on each board page. Hardware
acceptance requires UART evidence of board/SoC identity, DRAM bytes, EDID
result and selected mode, USB/keyboard status, payload EL, and
reset-to-MaskROM. Host tests do not replace that per-board sign-off.

For custom payload packaging, see [Firmware images, payloads, and device
trees](../payloads.md).

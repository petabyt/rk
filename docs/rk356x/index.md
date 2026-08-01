# RK356x devices

The RK356x implementation supports three explicit board targets:

- `roc3566`: Firefly ROC-RK3566-PC (`firefly,roc-rk3566-pc`).
- `yy3568`: Youyeetoo YY3568 (`youyeetoo,yy3568`).
- `rock3a`: Radxa ROCK 3A (`radxa,rock3a`).

The SoC code is shared in `src/rk356x`; board descriptors contain only identity,
LED and USB VBUS wiring, enabled OHCI companions, and connector notes. This does
not claim support for arbitrary RK3566/RK3568 boards.

YY3568 and ROCK 3A also have optional, isolated BL31/U-Boot variants for
handing PCIe, NVMe, removable SD, USB mass storage, eMMC, Linux, and EFI boot
to U-Boot. Their board manifests map the automatic order as NVMe, SD, USB, then
eMMC without treating onboard SDIO as an SD card. They do not change the
normal/demo images; ROC3566 does not inherit that support. See the
[chainloading guide](chainloading.md).

That optional variant can be packaged for the eMMC user area or onboard SPI
NOR without adding storage drivers to the bare-metal stage. RK3568 BootROM
loads the DDR blob and chainloader directly from the selected medium. SPI NOR
has immutable priority over eMMC, SD, and USB, so media installation and
restoration are deliberately guarded and remain board-qualified.

## Capability matrix

| Board | SoC | Normal/demo | USB-A OHCI | Chainloader | Persistent first-stage media |
| --- | --- | --- | --- | --- | --- |
| [ROC-RK3566-PC](boards/roc3566.md) | RK3566 | yes | OHCI0 | no | SD (normal/demo) |
| [YY3568](boards/yy3568.md) | RK3568 | yes | OHCI0, OHCI1 | vendor-FIT U-Boot | SD, eMMC, SPI NOR |
| [ROCK 3A](boards/rock3a.md) | RK3568 | yes | OHCI0, OHCI1 | mainline-FIT U-Boot | SD, optional eMMC, SPI NOR |

Shared RK356x drivers do not imply shared GPIO, storage, U-Boot, or memory
policy. Those properties are selected only by the board descriptor or the
validated chainloader manifest.

## Documentation map

| Topic | Page |
| --- | --- |
| BootROM delivery, normal/demo firmware, `jump_to_payload()`, memory, HDMI, and USB HID | [Common bare-metal firmware](bare-metal.md) |
| BL31/U-Boot FIT handoff, boot media, installation, OS discovery, EDK2, and OP-TEE | [Chainloading](chainloading.md) |
| Firefly-specific target, GPIOs, USB topology, artifacts, and validation | [ROC-RK3566-PC](boards/roc3566.md) |
| Youyeetoo-specific target, wiring provenance, U-Boot backend, and storage policy | [YY3568](boards/yy3568.md) |
| Radxa-specific target, mainline U-Boot backend, optional eMMC, and storage policy | [ROCK 3A](boards/rock3a.md) |

The generic pages describe only behavior implemented by shared RK356x code.
The board pages are authoritative for identity, connectors, GPIOs, supported
boot media, U-Boot policy, and hardware acceptance. A capability is not
inherited by another board merely because both use RK3566 or RK3568.

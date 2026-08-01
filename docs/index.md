# Rockchip bare-metal reference

This documentation covers two related uses of the repository:

1. Learning and experimenting with low-level Rockchip hardware bring-up.
2. Building practical first-stage firmware and board-specific boot artifacts.

Start with the page that matches your goal:

| Goal | Documentation |
| --- | --- |
| Build or run a board for the first time | [Getting started](intro.md) |
| Understand firmware-only, demo, SD delivery, payload, and DTB roles | [Firmware images, payloads, and device trees](payloads.md) |
| Compare supported RK3566/RK3568 boards | [RK356x devices](rk356x/index.md) |
| Understand RK356x display, input, payload handoff, and memory layout | [RK356x common bare-metal firmware](rk356x/bare-metal.md) |
| Boot an enabled RK3568 board through BL31/U-Boot and storage discovery | [RK356x chainloading](rk356x/chainloading.md) |
| Install or restore board-qualified SPI NOR/eMMC firmware | [Guarded installation](rk356x/chainloading.md#guarded-emmc-and-spi-nor-installation) |
| Understand possible EDK2 and OP-TEE integration | [Future EDK2 and OP-TEE integration](rk356x/chainloading.md#future-edk2-and-op-tee-integration) |
| Download or publish binary releases | [Binary releases](releases.md) |
| Study RK3399 or RK3588 internals | [Reference topics](ref.md) |

The repository source is available at <https://github.com/petabyt/rk>.

Hardware support is board-scoped. Read the board notes and recovery procedure
before writing persistent storage; a register map shared by two boards does
not imply identical GPIO, regulator, flash, or display wiring.

Copyright FUTO (C) 2025 FUTO

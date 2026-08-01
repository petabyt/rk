# Radxa ROCK 3A

| Property | Value |
| --- | --- |
| Build identifier | `rock3a` |
| SoC | RK3568 |
| Model | Radxa ROCK 3A |
| Compatible | `radxa,rock3a`, `rockchip,rk3568` |
| DDR loader | `rk3568_ddr_1560MHz_v1.25.bin` |
| UART | UART2 M0, 1,500,000 baud, 8N1 |
| User LED | GPIO0_B7, active high |
| USB host power | GPIO0_A6 host and GPIO0_D5 hub, active high |
| Bare-metal USB host | OHCI0 and OHCI1 |
| Chainloader backend | official mainline U-Boot FIT |
| BL33 load / stack limit | `0x00800000` / `0x03f00000` |
| First-stage media | MaskROM USB, SD, optional eMMC user area, SPI NOR |

The normal board descriptor enables both USB2 OHCI companions and the onboard
hub power rail. The OTG rail is outside the bare-metal USB-host contract and is
left untouched.

ROCK 3A identity and wiring are based on the upstream Linux/U-Boot device trees
and Armbian's `rock-3a.conf` at commit
`587b6f2c0a867859ca3f323f6008bee9e3ef1553`.

## Normal and demo artifacts

- `rock3a.bin`: normal firmware base requiring an appended FUEFI payload.
- `demo_rock3a.bin`: normal firmware plus the example payload.
- `rock3a.img`: RKNS v2 SD delivery image for the firmware-only base.
- `demo_rock3a.img`: standalone RKNS v2 SD demonstration image.

```sh
make usb BOARD=rock3a
make demo_rock3a.img
make maskrom3568  # SoC-wide optional xrock USB-plug flow
```

## Board-scoped U-Boot variant

The manifest pins official mainline U-Boot v2026.04 at commit
`88dc2788777babfd6322fa655df549a019aa1e69`, upstream
`rock-3a-rk3568_defconfig` and DTS, the board config fragment, Rockchip BL31
v1.46, and ROCK 3A-specific address and media policy. It does not modify
`rock3a.bin` or `demo_rock3a.bin`.

```sh
make chainload BOARD=rock3a
make chainload-check BOARD=rock3a
make usb-chainload BOARD=rock3a
```

After its three-second UART interruption window, mainline bootstd searches all
NVMe devices, removable SD (`mmc1`), USB mass storage, and eMMC (`mmc0`) in
that order. Onboard `mmc2` SDIO/Wi-Fi is excluded. SPI commands remain
interactive; SPI NOR can supply the first stage but is not an automatic OS
target. SATA variants are not enabled.

The eMMC is optional hardware. Installation must abort safely when it is not
populated. Read the common [chainloading and guarded installation
guide](../chainloading.md) before writing eMMC or SPI NOR. The exact
confirmation must use `rock3a`, and cross-board restores are rejected.

See [common bare-metal behavior](../bare-metal.md) for payload handoff, memory,
HDMI, input, and reset semantics.

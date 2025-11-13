# RK3588 bare metal bringup
Everything needed to bring up common hardware.

You will need the official rockchip-developed ddr image: https://github.com/rockchip-linux/rkbin/blob/master/bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.18.bin

See [bootrom.md](bootrom.md)

## Secure GRF init
A small sequence (100LoC) is needed to mark needed memory regions as secure. This is done primarily through the
main SGRF (Secure General Register File). There are actually a few SGRFs, and they are all undocumented.
This is necessary in order to use the VOP and setup a framebuffer.

## PMU
A few power domains need to be brought up in order to make some hardware accessible.
These onboard devices are completely powered off on reset. If you try to read/write to their MMIO addresses, you will get a translation error.

- PD_PHP
- PD_PCIE
- PD_GMAC
- PD_SECURE
- PD_VOP
- PD_VO0
- PD_VO1

Powering on a power domain is fairly well documented in the TRM. See `src/rk3588/pmu.c` for exactly how to do it.

Other hardware such as GIC, UART, and other essentials belong to the ALIVE power domain.
It is powered up on reset, and cannot be powered off.

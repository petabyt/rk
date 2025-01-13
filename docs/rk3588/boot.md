# RK3588 bare metal bringup
Everything needed to bring up common hardware.

You will need the official rockchip-developed ddr image: https://github.com/rockchip-linux/rkbin/blob/master/bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.18.bin

To make things going quickly at the start, I didn't do any uart setup and instead just piggybacked on top of what the ddr image brought up.

Regardless of boot medium, the bootrom will always jump to the OS in EL3 at 0x0. If you are running with arm-trusted-firmware then you will
be in EL2 - things will be a lot different then.

## MMU
- For the translation tables, I setup 16 1gb blocks, marking the 4th block as device memory.
- I set tcr_el3 to `0x1351c`.

## Secure GRF init
A small sequence (100LoC) is needed to mark needed memory regions as secure. This is done primarily through the
main SGRF (Secure General Register File). There are actually a few SGRFs, and they are all undocumented.
This is necessary to use the VOP and setup a framebuffer.

## PMU
A few power domains need to be brought up in order to make some hardware accessible.
These onboard devices are completely powered off on reset. If you try to read/write to their mmio, you will get a translation error.
- PD_PHP
- PD_PCIE
- PD_GMAC
- PD_SECURE
- PD_VOP
- PD_VO0
- PD_VO1
Powering on a power domain is fairly well documented in the TRM.

Other hardware such as GIC, UART, and other essentials belong to the ALIVE power domain.
It is powered up on with a reset, and cannot be powered off.

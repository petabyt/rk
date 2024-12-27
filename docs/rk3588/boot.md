# RK3588 bare metal boot sequence
Everything needed to bring up common hardware.

## MMU
.. Some notes on MMU setup ..

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

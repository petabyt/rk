# Bare Metal Rockchip Bootloader

This is a very minimal and bare bones bootloader/firmware for Rockchip devices based on the RK3399 and RK3588. Apart from partially implementing PSCI,
it brings up all hardware needed to get a framebuffer and a USB keyboard working. Unlike more robust bootloaders, it's booted directly by the bootrom and
uses custom DDR images to boot extremely quickly.

This project also includes some tips and info on bare metal Rockchip bringup, hopefully shedding some light on these not so well documented SoCs.

This project implements:
- RK3399 VOP/CRU/GPIO/timer driver
- RK3588 VOP2/CRU/IOC/GPIO/PWM/PMU driver
- Analogix eDP driver
- OHCI driver (not finished)
- A few PSCI commands

External dependencies included:
- HDMI/eDP Combo PHY (HDPTX) driver from Linaro
- Proprietary DDR and maskrom images from Rockchip

Supported devices:  
- Pinebook Pro
- Cool-Pi Genbook
- (partial Orange Pi 5 support, no HDMI working yet)

# Compiling
```
sudo apt install gcc-aarch64-linux-gnu libusb-1.0-0-dev make
make all
```

There's a bunch of targets, but this is the gist:
- `pinebook-ddr.bin`: Customized DDR image for the Pinebook.
- `pinebook.bin`: Bootloader (SPL) binary, loaded to 0x0
- `demo.bin`: Payload demo binary, bootloader jumps to this in EL2
- `demo_pinebook.bin`: `demo.bin` appended to `pinebook.bin`

# Thanks
- Colt Judice
- Hans Jorgensen
- Hannes Bredberg

Copyright FUTO (C) 2025 FUTO

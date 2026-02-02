# Bare Metal Rockchip Bootloader

This is a very minimal and bare bones bootloader/firmware for Rockchip devices based on the RK3399 and RK3588.
It brings up all hardware needed to get a framebuffer and a USB keyboard working as fast as possible.

This project also includes some tips and info on bare metal Rockchip bringup, hopefully shedding some light on these not so well documented SoCs.

This project implements:
- RK3399 VOP/CRU/GPIO/timer driver
- RK3588 VOP2/CRU/IOC/GPIO/PWM/PMU driver
- Analogix eDP driver
- OHCI driver (not finished)
- DDR image wrappers
- Some PSCI commands + additional UEFI-like commands
- libusb driver for USB-OTG maskrom mode
- SD image packer for rk3588/rk3399

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
- `demo_pinebook.img`: Bootable SD card image for the Pinebook

# Thanks
- Colt Judice
- Hans Jorgensen
- Hannes Bredberg

Copyright FUTO (C) 2025 FUTO

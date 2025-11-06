# Bare Metal Rockchip Bootloader

This is a bare metal bootloader for Rockchip devices based on the RK3399 and RK3588. It brings up all hardware needed to get
a framebuffer and a USB keyboard working. This bootloader is booted directly by the bootrom, so it bypasses earlier boot stages like ATF and U-Boot.

This project also includes some documentation about obscure rockchip info, hopefully shedding some light on these not so well documented SoCs.

This project implements:
- RK3399 VOP/CRU/GPIO/timer driver
- RK3588 VOP2/CRU/IOC/GPIO/PWM/PMU driver
- Analogix eDP driver
- OHCI driver (not finished)

External dependencies included:
- HDMI/eDP Combo PHY (HDPTX) driver from Linaro
- Proprietary DDR and maskrom images from Rockchip

## Thanks
- Colt Judice
- Hans Jorgensen
- Hannes Bredberg

Copyright FUTO (C) 2025 FUTO

# References
## RK3588
- edk2 uefi monorepo: https://gitlab.com/rk3588_linux/rk/uefi-monorepo
(Lots of early bare-metal rk3588 code written by rockchip employees, good reference for when the TRM is missing info)
- rkdocs: https://github.com/axlrose/rkdocs
- TRM/Datasheet: https://github.com/FanX-Tek/rk3588-TRM-and-Datasheet
- RK3588 Base: https://github.com/torvalds/linux/blob/master/arch/arm64/boot/dts/rockchip/rk3588-base.dtsi pinctrl https://github.com/torvalds/linux/blob/master/arch/arm64/boot/dts/rockchip/rk3588-base-pinctrl.dtsi

## RK3399
- RK3399 TRM v1.4: https://opensource.rock-chips.com/images/e/ee/Rockchip_RK3399TRM_V1.4_Part1-20170408.pdf
- RK3399 TRM V1.3 Part2: https://rockchip.fr/Rockchip%20RK3399%20TRM%20V1.3%20Part2.pdf
- 2022 Pinebook Pro main board schematic: https://files.pine64.org/doc/PinebookPro/pinebookpro_schematic_v21a_20220419.pdf
- Tool to decode exception syndrome register: https://esr.arm64.dev/
- Intro to eDP: https://www.vesa.org/wp-content/uploads/2010/12/DisplayPort-DevCon-Presentation-eDP-Dec-2010-v3.pdf
- eDP technical overview: https://www.vesa.org/wp-content/uploads/2011/01/ICCE-Presentation-on-VESA-DisplayPort.pdf
- DisplayPort Specification (covers DPCD): https://glenwing.github.io/docs/DP-1.1.pdf
- rk3399 bootrom analysis: https://github.com/trebisky/Rockchip/blob/4c4623e3df37f01d1d1ae0131fec5bc75f9e8463/bootrom/bootrom.txt
- arm-trusted-firmware: https://github.com/ARM-software/arm-trusted-firmware
- Dynamic Memory Controller: https://www.kernel.org/doc/Documentation/devicetree/bindings/devfreq/rk3399_dmc.txt
- ARM GIC architecture specification: https://developer.arm.com/documentation/ihi0069/latest/

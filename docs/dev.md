# Development
The best way to setup a rockchip device for bare-metal development is by enabling the 'maskrom' interface that allows custom images to be loaded
over an OTG port.

In order to get this working, you have to make all bootable mediums un-bootable by the bootrom. This includes emmc, any SPI flash, or
sdmmc. You don't have to completely erase these devices, you just need to erase the rockchip magic at the start (`RKNS` or `RK33`)

This wiki has instructions for how to do this on various rockchip devices, see Devices/

For most devices you will need a usb-c to usb-a data cable to plug it into your dev machine. In other cases you will need a usb-a to usb-a
cable.

Once you get your rockchip device setup and plugged in (check `dmesg -w`), you will need a tool to send the images over USB.
[xrock](https://github.com/xboot/xrock) is probably the easiest to install and use.

For RK3399 devices:
```
xrock <ddr_img> <os_img>
```
For RK3588 devices:
```
xrock <ddr_img> <os_img> --rc4-off
```

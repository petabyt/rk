# Getting started

## Development
The best way to setup a rockchip device for bare-metal development is by enabling the 'maskrom' interface that allows custom images to be loaded
over an OTG port.

In order to get this working, you have to make all bootable mediums un-bootable by the bootrom. This includes emmc, any SPI flash, or
sdmmc. You don't have to completely erase these devices, you just need to erase the rockchip magic at the start (`RKNS` or `RK33`)

This wiki has instructions for how to do this on various rockchip devices, see Devices/

For most devices you will need a usb-c to usb-a data cable to plug it into your dev machine. In other cases you will need a usb-a to usb-a
cable.

Once you get your rockchip device setup and plugged in (check `dmesg -w`), you will need a tool to send the images over USB.
This repo includes a tool (`make rock.out`) that can do this. You can also use [xrock](https://github.com/xboot/xrock).

For RK3399 devices:
```
xrock <ddr_img> <os_img>
./rock.out --v1 --ddr <ddr_img> --os <os_img>
```
For RK3588 devices:
```
xrock <ddr_img> <os_img> --rc4-off
./rock.out --v2 --ddr <ddr_img> --os <os_img>
```

## How to program Rockchip hardware

The best way to learn how to program certain Rockchip hardware is study the its device tree file.
A device tree describes how all the hardware in a device is wired and setup, and how to configure it (turning on GPIO pins, changing iomux functions)
It will map devices such as fans, LEDs, I2C devices, screens, any other non-enumerable hardware.

For example, here's the DTS file for the Genbook: https://github.com/torvalds/linux/blob/master/arch/arm64/boot/dts/rockchip/rk3588-coolpi-cm5-genbook.dts

Schematics, if available, are also just as useful.

## Memory

On the RK3588, physical memory from `0x0`-`0xa00000` appears to be reserved for
secure world only. If you try and execute code in non-secure state (EL0-EL2) then
you'll get a '32-bit instruction trap' exception brought up in EL3.

The RK3399 has the same situation but the range is `0x0`-`0x200000`.

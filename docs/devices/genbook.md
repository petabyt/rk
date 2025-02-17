# Genbook / Cool-pi notebook
Photos: https://www.flickr.com/photos/201609787@N08/albums/72177720322040367/

The Genbook comes with U-boot SPL on the SPI flash, you can make it unbootable like so:
```
printf '\x00\x00\x00\x00' | dd of=/dev/mtdblock0 bs=1 seek=$((0x10000)) count=4 conv=notrunc
printf '\x00\x00\x00\x00' | dd of=/dev/mtdblock0 bs=1 seek=$((0x60000)) count=4 conv=notrunc
```
The boot image in SPI flash must have two copies (at `0x10000` and `0x60000`) so we need to cripple both
or the bootrom will load the unmodified copy.

The bootrom maskrom mode exposes itself on the left side USB-C port.

To boot back into u-boot:
```
xrock maskrom rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin u-boot-genbook.bin --rc4-off
```

The 'loader key' button on the bottom will disable all boot methods (SPI, emmc) and get the bootrom into maskrom mode.

In order to erase the emmc from maskrom mode:
```
xrock maskrom rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin rk3588_usbplug_v1.11.bin --rc4-off
xrock flash erase 0 100000
```
in order to erase the SPI:
```
xrock maskrom rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin rk3588_usbplug_v1.11.bin --rc4-off
# I didn't see an option to erase loader/SPI flash, so I just wrote a random 30mb file.
rkdeveloptool wl 0 '/home/daniel/Downloads/fpupdate-output-xa2-loader.bin'
```

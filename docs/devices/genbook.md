# Genbook / Cool-pi notebook
Photos: https://www.flickr.com/photos/201609787@N08/albums/72177720322040367/

The Genbook comes with U-boot SPL on the SPI flash, you can make it unbootable like so:
```
printf '\x00\x00\x00\x00' | dd of=/dev/mtdblock0 bs=1 seek=$((0x10000)) count=4 conv=notrunc
printf '\x00\x00\x00\x00' | dd of=/dev/mtdblock0 bs=1 seek=$((0x60000)) count=4 conv=notrunc
```
The boot image in SPI flash must have two copies (at `0x10000` and `0x60000`) so we need to cripple both
(or the bootrom will load the unmodified copy)

The bootrom exposes itself on the left side USB-C port.

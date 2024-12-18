## RK3399 Boot process
All cores of all clusters enter the bootrom on reset. The first few instructions of the bootrom look like this:
```c
// sram offsets - these are different for rk33xx and rk35xx chips
uint32_t *sram_a = (uint32_t *)0xffff00e0;
uint32_t *sram_b = (uint32_t *)0xffff00e8;
if (mpidr_el1 != 0) {
	while (*sram_a != 0xdeadbeaf) {
		asm("wfe");
	}
	typedef void entry(void);
	((entry *)(*sram_b))();
	// undefined
}
```

The bootrom supports booting over SPI, eMMC, SDMMC, and USB-OTG. All of these boot methods load both a DDR image (blob that sets up the memory controller) and an OS image.
```c
// ... (clock setup, ddr stuff, efuse, etc) ...
// Try all boot methods
// each boot method has check(), load_ddr(), and load_main() functions.
try_boot_methods();
// Bring up the USB3 DWC3 controller into device mode and start accepting control transfers
dwc3_usb_spin();
while (1);
```

For USB-OTG, the bootrom exposes itself as a device on a USB-C (or USB-A) port and accepts control transfers for both images.  
`Demos/Rock.fut` implements how these control transfers work.
For all other boot methods, an image with an ID block (at offset `0x8000`) is required, which tells the bootrom at what offset
the DDR/OS images are, as well as other settings (see `FUBS/MakeBoot/Rockchip.fut`)

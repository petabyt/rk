# RK3588 bootrom function
```
if (mpidr_el1 & 0xffff) != 0 {
	((uint32_t)0xffff00e0)[0] = 1;
	while (((uint32_t)0xffff00e0)[0] != 0xdeadbeaf) {
		__asm__("wfe");
	}
	typedef core_entry();
	core_entry *entry = (core_entry *)(uintptr_t)(((uint32_t)0xffff00e8)[0]);
	entry();
}
boot();
```

## boot()
The main boot function does a lot of stuff, mostly with undocumented mmio registers:
- ddrphy
- cru
- crypto
- saradc

After that it runs through a loop bringing up and trying various storage devices to find a bootable medium.
1. FSPI 0
2. FSPI 1
3. FSPI 2
4. FSPI 3
5. FSPI 4
6. FSPI 6
7. eMMC
8. sdmmc

TODO: Document iomux settings for all these devices

If no bootable medium is found, it will bring up a dwc3 gadget on USB3OTG_0.

This is a very small bootable demo that can run on the Pinebook Pro. From bare metal, it will initialize and
power up everything needed to get the 1080p eDP display working. Feel fee to tinker with it.

Some basic RK3588 support has been added, but is not nearly as complete as the RK3399 code.

Any pinouts or non-enumerable devices are hardcoded and the code must be modified to support alternative devices.
```
make os3399.bin
make os3588.bin
```

Usage:
- Disable eMMC
- Plug in device with USB-C (as if it were a camera or phone)
- Install [xrock](https://github.com/xboot/xrock)
- `make`

For more questions: daniel@futo.org
Copyright (C) FUTO 2023-2024  

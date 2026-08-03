# Radxa ROCK 3A

| Property | Value |
| --- | --- |
| Build identifier | `rock3a` |
| SoC | RK3568 |
| Model | Radxa ROCK 3A |
| Compatible | `radxa,rock3a`, `rockchip,rk3568` |
| User LED | GPIO0_B7, active high |
| USB host power | GPIO0_A6, active high |
| USB hub power | GPIO0_D5, active high |
| Enabled USB2 hosts | OHCI0 and OHCI1 |

Build or load the standalone demo with:

```sh
make demo_rock3a.img
make usb_rock3a
```

The normal artifacts are `rock3a.bin` and `rock3a.img`; both require an
appended compatible payload. `demo_rock3a.bin` and `demo_rock3a.img` include
the example payload.

Hardware validation of UART2, HDMI, USB host and hub power, SD delivery and
reset is still required before this board is considered tested.

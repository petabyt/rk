# RK356x bare-metal support

The RK356x platform supports three explicit boards:

- [Firefly ROC-RK3566-PC](boards/roc3566.md)
- [Youyeetoo YY3568](boards/yy3568.md)
- [Radxa ROCK 3A](boards/rock3a.md)

Shared SoC code initializes DDR discovery, UART2 M0, the RK356x memory map,
VOP2, HDMI, USB2 OHCI and the existing firmware-call interface. Board files
contain only identity and wiring such as LEDs, USB host power and enabled OHCI
companions. This does not claim support for arbitrary RK3566 or RK3568 boards.

The boot flow remains the same bare-metal model used elsewhere in this project:

```text
RK356x BootROM -> Rockchip DDR blob -> rk firmware -> appended EL2 payload
```

Normal firmware artifacts require a compatible payload appended after the
firmware image. Demo artifacts already contain the repository's example
payload and are the appropriate standalone hardware test.

## Common behavior

- UART2 M0 runs at 1,500,000 baud, 8N1.
- HDMI reads EDID once at boot and accepts progressive RGB modes through 4K30.
- An attached display with unusable EDID falls back to 1280x720p60.
- A board without HDMI HPD continues headless and reports no framebuffer.
- Enabled USB2 OHCI ports poll directly attached HID boot keyboards.
- `FU_GET_CHAR` and `FU_POLL_CHAR` expose keyboard input to the payload.
- Reset and shutdown firmware calls return the SoC to MaskROM.

USB hubs, high-speed host controllers, runtime display hotplug and storage
access are outside the RK356x firmware implementation.

## Memory layout

| Range | Purpose |
| --- | --- |
| `0x00a00000` | Relocated EL2 payload |
| `0x07ff0000-0x08000000` | EL3 and EL2 stacks |
| `0x08000000-0x08400000` | Non-cacheable OHCI, DTB and firmware exchange area |
| `0x10000000-0x12000000` | Non-cacheable framebuffer arena |
| `0xf0000000-0xffffffff` | Device-mapped MMIO |

DRAM size is read from the RK356x DDR ATAG when available, then from PMUGRF
geometry, with a conservative 1 GiB fallback.

## Artifacts and loading

Build every existing and RK356x target with:

```sh
make all
```

For a first test, load the board's demo directly into RAM:

```sh
make usb_roc3566
make usb_yy3568
make usb_rock3a
```

After the board has been placed in MaskROM with its hardware recovery control,
the optional SoC-level xrock USB-plug helpers are:

```sh
make maskrom3566
make maskrom3568
```

`maskrom3566` uses the RK3566 DDR blob. `maskrom3568` is shared by the two
supported RK3568 boards. These helpers upload the USB-plug loader; they are not
a prerequisite for the direct `usb_<board>` flow. Direct firmware loading must
use the board-qualified target so board wiring cannot be selected by SoC name
alone.

Alternatively, write the corresponding `demo_<board>.img` to a complete SD
device. The normal `<board>.img` contains firmware only and still requires an
appended payload.

Hardware validation remains pending until each board has a saved UART
transcript covering identity, DDR size, display mode, USB keyboard state,
payload EL and reset.

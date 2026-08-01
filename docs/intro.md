# Getting started

## Decide what you want to run

The repository produces three intentionally separate firmware styles:

| Style | Purpose | Storage awareness |
| --- | --- | --- |
| Normal bare-metal firmware | Developer base that initializes hardware and requires an appended EL2 payload | No PCIe/NVMe/filesystem drivers |
| Demo firmware | Standalone smoke test with the repository's example payload appended | No PCIe/NVMe/filesystem drivers |
| Board-scoped RK3568 chainloader | Validate a FIT, enter BL31, and start the selected board's U-Boot at EL2 | U-Boot—not the first stage—handles storage |

Choose the normal/demo images when developing firmware services or an EL2
payload. Choose a supported RK3568 chainloader when the goal is Linux,
extlinux, or an EFI application on NVMe, SD, USB mass storage, or eMMC.

Read [Firmware images, payloads, and device trees](payloads.md) before choosing
an SD artifact. In particular, a normal firmware-only `.img` has no payload and
halts with `Bad payload magic`; it is not the standalone version of the demo.

## Use a release or build from source

A release archive is the simplest option when you only need board binaries.
Download the archive and top-level `SHA256SUMS`, verify it, then verify the
archive's internal checksums after extracting:

```sh
grep 'yy3568.tar.xz' SHA256SUMS | sha256sum -c -
tar -xJf rk-v1.2.3-yy3568.tar.xz
cd rk-v1.2.3-yy3568
sha256sum -c SHA256SUMS
```

Build from source when modifying firmware, U-Boot policy, or board support:

```sh
sudo apt install gcc-aarch64-linux-gnu libusb-1.0-0-dev make xxd \
  device-tree-compiler cpp python3 xz-utils
make all
make SHELL=/bin/bash check -j"$(nproc)"
```

The normal build uses vendored Rockchip inputs and is offline. The optional
chainloader build needs the pinned U-Boot source unless `UBOOT_SRC` or a
compatible `UBOOT_ITB` is supplied locally.

## Start with RAM-only MaskROM loading

Direct USB loading is the safest first hardware test because it does not write
SPI, eMMC, SD, or NVMe. Connect the board's OTG port to the build host, enter
MaskROM using the board's documented recovery control, and verify that exactly
one Rockchip device appears.

Do not erase every boot medium as a first step. Rockchip BootROM normally tries
persistent media before USB, but supported boards provide a hardware recovery
method for forcing MaskROM. Invalidation or restoration of persistent firmware
should be a deliberate recovery action, not routine setup.

Build and load the demo matching the board:

```sh
make usb3399    # Pinebook Pro
make usb3588    # Genbook
make usb3566    # ROC-RK3566-PC
make usb BOARD=yy3568
make usb BOARD=rock3a
```

`rock.out` sends the DDR image through BootROM command `0x471` and the firmware
or demo through `0x472`. The image runs from RAM and disappears after reset.
The `maskrom3566` and `maskrom3568` targets provide the alternative xrock
USB-plug flow.

## Connect serial before debugging display or boot

Serial output is the primary evidence for early firmware:

| Target | UART setting |
| --- | --- |
| RK3566/RK3568 normal and chainloader firmware | UART2 M0, 1,500,000 baud, 8N1 |
| Existing RK3399/RK3588 helpers | 115,200 baud unless the board guide says otherwise |

For RK356x:

```sh
make uart2
# equivalent: screen /dev/ttyUSB0 1500000
```

Expected RK3568 chainloader output includes board/SoC identity,
`source=usb`, `source=sd`, `source=emmc`, or `source=spi-nor`, validation
status, and the BL31/U-Boot transition.

## Boot a normal or demo SD image

For a standalone first boot, build the demo for the selected board:

```sh
make demo_roc3566.img
make demo_yy3568.img
make demo_rock3a.img
```

Write the complete `.img` with a trusted imaging tool. The destination is the
whole SD device, not a partition, and existing contents will be overwritten.
Verify the device identity and size with `lsblk` before writing.

Rockchip BootROM reads the RKNS image, runs its DDR loader, and copies the
firmware plus any appended payload into RAM. This does not require an SD/MMC
driver in the firmware, and the firmware cannot read files from the card after
BootROM hands over control.

Demo images append the repository's example EL2 payload and are the correct
initial hardware test. Normal `.bin` and `.img` artifacts contain firmware
only. They unconditionally look for an appended FUEFI payload, report
`Bad payload magic`, and halt when run alone. To use one, append a compatible
payload to the `.bin` and regenerate the RKNS image so its header covers the
combined binary. See [the custom payload workflow](payloads.md#build-a-custom-payload-image).

## Boot an enabled RK3568 board into U-Boot

For a first chainloader test, stay RAM-only:

```sh
make chainload BOARD=rock3a
make chainload-check BOARD=rock3a
make usb-chainload BOARD=rock3a
```

At the U-Boot prompt, basic evidence is:

```text
pci enum
nvme scan
nvme info
part list nvme 0
```

Once that works, choose SD, eMMC, or SPI NOR for autonomous first-stage boot.
Replace `rock3a` with `yy3568` to select that board's independent manifest.
Read [RK356x chainloading](rk356x/chainloading.md) before installation. SPI NOR has
higher immutable BootROM priority than eMMC, SD, and ordinary USB fallback.

## Recovery model

- Direct MaskROM runs only in RAM and cannot damage persistent storage.
- An invalid chainloader FIT logs an error and requests reset-to-MaskROM.
- The guarded installer requires a new backup directory before any write.
- eMMC installation preserves LBA 0-63 and writes only the ID block beginning
  at LBA `0x40`, after rejecting MBR/GPT partition overlap.
- SPI installation backs up the complete detected NOR and writes only the
  required range at offset zero.
- Every persistent write is read back and SHA-256 verified before reset.

Keep backups outside the target device. A backup stored only on the device
being modified is not a recovery copy.

## Learn from DTS and schematics

Device trees describe non-enumerable board wiring: GPIOs, pinmux, regulators,
clocks, buses, displays, LEDs, and resets. Schematics and vendor DTS files are
the starting point for a new board, but they must be checked against actual
hardware and the relevant TRM.

A DT node is descriptive data, not a driver. BootROM does not consume this
project's DTB, and listing a USB or MMC controller does not make it operational
in the firmware. A payload may rely on a node only when the node is complete,
the firmware has prepared the hardware as required, and the payload has a
matching driver. The minimal firmware DTBs are not Linux board DTBs.

Shared SoC support is not enough to enable a board. A new target needs an
explicit descriptor or chainloader manifest, reviewed memory ranges and
pinmux, isolated objects, tests, and physical hardware sign-off.

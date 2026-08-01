# RK356x BL31/U-Boot chainloading

The optional chainloader turns the minimal RK firmware into a practical first
stage without teaching it PCIe, NVMe, filesystems, Linux boot, or EFI. U-Boot
owns those jobs:

```text
RK3568 BootROM -> Rockchip DDR blob -> board chainloader -> BL31 -> U-Boot EL2
       ^ SPI NOR, eMMC, SD, or MaskROM USB                    -> NVMe / SD / USB / eMMC
                                                               -> Linux or EFI app
```

This is a board-scoped variant. Normal `yy3568.bin`, `rock3a.bin`, and demo
images keep their FUEFI payload behavior. ROC3566, RK3399, and RK3588 targets
do not inherit RK3568 storage, address, GPIO, or U-Boot policy.

## Supported chainloader boards

| `BOARD` | U-Boot backend | Source pin | BL33 load / stack limit | Automatic OS targets |
| --- | --- | --- | --- | --- |
| [`yy3568`](boards/yy3568.md) | Radxa vendor FIT | `39cd993e5d6296635438e84f4576b3a9bf76f86e` | `0x00a00000` / `0x00c00000` | `nvme0 nvme1` -> `mmc1` -> `usb0` -> `mmc0` |
| [`rock3a`](boards/rock3a.md) | official mainline FIT | v2026.04, `88dc2788777babfd6322fa655df549a019aa1e69` | `0x00800000` / `0x03f00000` | `nvme` -> `mmc1` -> `usb` -> `mmc0` |

Both use Rockchip BL31 v1.46 from rkbin commit
`ecb4fcbe954edf38b3ae037d5de6d9f5bccf81f4`. ROCK 3A uses upstream
`rock-3a-rk3568_defconfig` and DTS; Armbian commit
`587b6f2c0a867859ca3f323f6008bee9e3ef1553` records the board-selection
reference. YY3568 retains its vendor backend and locally reviewed board
overlay. A backend change for either board is a separate port, not a shared
default.

## Building one board

Use the generic interface and always name the board:

```sh
make chainload BOARD=rock3a
make chainload-check BOARD=rock3a
make usb-chainload BOARD=rock3a
```

Replace `rock3a` with `yy3568` for that board. `usb3568-uboot` remains a
deprecated YY3568 compatibility alias; new scripts should use the qualified
command.

Each build creates:

- `<board>-u-boot.itb`: U-Boot, its control DTB, and split BL31 segments.
- `uboot_<board>.bin`: dedicated first stage followed by that FIT.
- `uboot_<board>.img`: whole-SD image with its ID block at LBA `0x40`.
- `uboot_<board>_idbloader.img`: raw RKNS v2 ID block for eMMC LBA `0x40`.
- `uboot_<board>_spi.img`: BootROM SPI-NOR layout for offset zero.
- `<board>-u-boot-source.tar.xz`: patched, buildable corresponding source.

The default build fetches the manifest-pinned U-Boot commit. To use an existing
Git tree containing that exact commit:

```sh
make chainload BOARD=rock3a UBOOT_SRC=/path/to/u-boot
```

The source is never patched in place. A clean snapshot is exported to
`build/chainload/<board>/source`, and only that board's overlay is applied.
Generated headers, objects, tools, source snapshots, links, and stamps stay
under the same board namespace.

For offline FIT iteration, an existing compatible FIT can build just the
combined binary:

```sh
make uboot_rock3a.bin UBOOT_ITB=/path/to/u-boot.itb
```

Persistent-media generation also needs the pinned source build's `mkimage` or
an explicitly supplied compatible one:

```sh
make chainload-media BOARD=rock3a MEDIA=emmc MKIMAGE=/path/to/tools/mkimage
make chainload-media BOARD=rock3a MEDIA=spi-nor MKIMAGE=/path/to/tools/mkimage
```

U-Boot `rksd` creates the SHA-256 RKNS v2 ID block. `rkspi` spreads those
bytes into the first 2 KiB of each 4 KiB SPI region. These rules do not use or
modify the normal firmware's `makeboot.out` path.

## BootROM priority and source evidence

RK3568 BootROM searches immutable media in this order:

```text
SPI NOR -> SPI NAND -> parallel NAND -> eMMC -> SD -> MaskROM USB
```

This project supports SPI NOR and the eMMC user area for the two boards above;
SPI NAND, parallel NAND, and eMMC boot partitions are out of scope. Valid SPI
firmware wins over valid eMMC or SD firmware and prevents normal USB fallback.
Restore or invalidate SPI before testing eMMC fallback.

The common RK3568 stage reads the BootROM source word at `0xfdcc0010` before
BL31 can reuse that SRAM and logs `source=spi-nor`, `source=emmc`, `source=sd`,
or `source=usb` over UART2 at 1.5 Mbaud.

Direct MaskROM loading remains the safest development and recovery path:

```sh
make usb-chainload BOARD=yy3568
make usb-chainload BOARD=rock3a
```

Use the selected board's documented recovery control when valid persistent
firmware prevents ordinary USB discovery. Confirm exactly one RK356x device
before continuing.

## Guarded eMMC and SPI-NOR installation

The installer is compatible with xrock commit
`b90d3ba8f0a48320e3888701f7e66e0e4e038bbb` and rkdeveloptool commit
`304f073752fd25c854e1bcf05d8e7f925b1f4e14`. Install host tools separately;
release archives never bundle them. Follow
[Prepare a Linux programming host](../intro.md#prepare-a-linux-programming-host)
to build the pinned tools, configure restricted udev access, and verify the
MaskROM-to-loader transition before attempting a persistent write.

### SPI NOR to NVMe walkthrough

SPI NOR contains the RK3568 first-stage firmware, BL31, and U-Boot. It does
not contain Linux or a root filesystem. Before installing it, prepare the NVMe
with extlinux, `boot.scr`, or an AArch64 EFI application as described in
[U-Boot automatic OS discovery](#u-boot-automatic-os-discovery).

#### 1. Build and validate the SPI image

For ROCK 3A:

```sh
make chainload BOARD=rock3a -j"$(nproc)"
make chainload-check BOARD=rock3a
```

This produces the SPI image and its two principal inputs:

```text
uboot_rock3a_spi.img
uboot_rock3a.bin
rock3a-u-boot.itb
```

For YY3568:

```sh
make chainload BOARD=yy3568 -j"$(nproc)"
make chainload-check BOARD=yy3568
```

The corresponding outputs are `uboot_yy3568_spi.img`,
`uboot_yy3568.bin`, and `yy3568-u-boot.itb`.

#### 2. Put the board into MaskROM mode

Power off the board and use its documented recovery/MaskROM control while
connecting the appropriate USB OTG port to the host. Confirm detection before
writing anything:

```sh
rkdeveloptool ld
```

There must be exactly one device similar to:

```text
Vid=0x2207 Pid=0x350a Mode=Maskrom
```

If valid firmware already exists in SPI NOR, BootROM selects it before eMMC,
SD, or USB. In that case, force MaskROM with the board's recovery control. The
installer also accepts a single RK356x device that is already in loader mode.

#### 3. Flash SPI NOR with a complete backup

The backup path must be absolute, new or empty, and have enough free space for
the complete SPI NOR contents. For ROCK 3A:

```sh
make flash-chainload \
  BOARD=rock3a \
  MEDIA=spi-nor \
  BACKUP_DIR=/absolute/path/rock3a-spi-backup \
  CONFIRM=rock3a:spi-nor
```

For YY3568:

```sh
make flash-chainload \
  BOARD=yy3568 \
  MEDIA=spi-nor \
  BACKUP_DIR=/absolute/path/yy3568-spi-backup \
  CONFIRM=yy3568:spi-nor
```

From a release archive, invoke the packaged utility directly, for example:

```sh
./install/flash-chainload.sh flash rock3a spi-nor \
  /absolute/path/rock3a-spi-backup rock3a:spi-nor
```

The guarded installer:

1. Verifies that exactly one RK356x device is attached.
2. Loads the manifest-selected DDR and USB-plug helpers into RAM.
3. Selects SPI NOR with `rkdeveloptool cs 9`.
4. Validates the JEDEC identification and detected capacity.
5. Backs up the complete chip as `complete-spi-nor.bin`.
6. Writes only `uboot_<board>_spi.img` at offset zero.
7. Reads the written range back and compares it byte-for-byte.
8. Resets the board only after successful verification.

Do not use `rkdeveloptool ef`; the installer intentionally avoids erasing the
complete chip. A verification mismatch leaves the board in loader mode and
prints the exact restore command. Keep `backup.json`, the binary backups, and
`SHA256SUMS` together.

#### 4. Confirm NVMe boot

Connect UART2 at 1,500,000 baud, 8N1. After reset, the first stage should log:

```text
source=spi-nor
```

U-Boot then provides a three-second interruption window and searches:

```text
NVMe -> removable SD card -> USB mass storage -> eMMC
```

If U-Boot starts but does not find the operating system, use `pci enum`,
`nvme scan`, `nvme info`, and `part list nvme 0` at its prompt. That indicates
an NVMe discovery or boot-content problem rather than an SPI installation
problem.

### eMMC installation

Use a new, empty, absolute backup directory and an exact board-qualified
confirmation. For ROCK 3A:

```sh
make flash-chainload BOARD=rock3a MEDIA=emmc \
  BACKUP_DIR=/absolute/path/rock3a-emmc-backup CONFIRM=rock3a:emmc
```

The same command works for `BOARD=yy3568` with a matching path and
`CONFIRM=yy3568:emmc`. From a release archive, call
`install/flash-chainload.sh flash` with the same four arguments.

The utility loads the manifest-selected DDR and USB-plug helpers, requires
exactly one RK356x device, and selects eMMC with `rkdeveloptool cs 1`.

It backs up LBA 0-63 and the complete destination range, parses MBR and GPT
metadata, rejects a partition crossing the ID block, and writes only the raw
ID block at LBA `0x40`. It never replaces the partition table. Every write is
read back sector-for-sector and compared byte-for-byte before reset.

### Restoring a backup

Restore using the board and medium recorded in `backup.json`:

```sh
make restore-chainload BACKUP=/absolute/path/rock3a-spi-backup \
  CONFIRM=restore:rock3a:spi-nor
```

Restoration rejects a backup whose board, medium, manifest SHA-256, storage ID,
write range, or detected capacity differs from the selected target. This stops
a YY3568 backup from being restored to ROCK 3A, or vice versa.

## FIT and handoff policy

The common first stage copies the FIT to `0x08000000-0x08400000` before
loading a segment. It requires inline, uncompressed ARM64 images with SHA-256,
rejects external data and BL32/OP-TEE, and permits only:

- BL31 DRAM segments in `0x00040000-0x00200000`.
- BL31 SRAM segments in `0xfdcc0000-0xfdcf0000`.
- BL33 at the selected board's manifest address, ending before that board's
  initial stack.

The stage reproduces U-Boot FIT metadata and control-DTB placement, constructs
TF-A v1 parameters for a non-secure AArch64 EL2 BL33, cleans loaded ranges,
tears down EL3 caches/MMU, and branches to BL31. Validation or preparation
errors are logged and reset to MaskROM.

It deliberately omits HDMI, framebuffer, OHCI/HID, FUEFI, storage, and demo
code. SHA-256 detects accidental corruption; this is not authenticated boot.

## U-Boot automatic OS discovery

Each board gets a three-second UART interruption window and then scans OS media
in this order:

```text
NVMe -> removable SD card -> USB mass storage -> eMMC
```

The order is common policy, but the U-Boot targets are board-scoped. YY3568's
vendor distro backend uses `nvme0 nvme1 mmc1 usb0 mmc0`. ROCK 3A's mainline
bootstd backend uses `nvme mmc1 usb mmc0`, where the unnumbered class names
scan all devices in that class. On both boards `mmc1` is the removable SD slot
and `mmc0` is eMMC. `mmc2` is onboard SDIO/Wi-Fi and is intentionally not
treated as boot storage.

Each target searches supported partitions for extlinux, `boot.scr`, or
`EFI/BOOT/BOOTAA64.EFI` and falls through when no valid bootflow succeeds.
USB mass-storage support is enabled for both backends. SPI commands remain
interactive, and automatic network or SATA boot is not added.

This U-Boot OS order is separate from the immutable BootROM firmware-source
order described above. For example, SPI NOR may supply the chainloader while
U-Boot subsequently loads Linux from SD, USB, or eMMC. ROCK 3A retains upstream
PCIe supplies/resets, JEDEC NOR through SFC, and 8-bit/HS200 eMMC definitions.
YY3568 retains its reviewed vendor definitions. Armbian's ROCK 3A SATA variants
remain excluded.

The artifacts do not contain Linux or a root filesystem. Prepare any automatic
medium with a kernel `Image`, the Linux DTB for the actual board, and optionally
an initramfs plus `/extlinux/extlinux.conf`, or place an AArch64 EFI application
at `EFI/BOOT/BOOTAA64.EFI` on a FAT EFI System Partition. Never substitute the
minimal bare-metal DTB or U-Boot control DTB for the Linux board DTB.

Useful prompt checks are:

```text
pci enum
nvme scan
nvme info
part list nvme 0
sf probe
mmc list
mmc info
```

## Future EDK2 and OP-TEE integration

EDK2 and OP-TEE have different roles in the trusted-firmware boot model. EDK2
is normal-world boot firmware and would replace U-Boot as BL33. OP-TEE is an
optional secure-world operating system loaded as BL32 beneath BL31:

```text
RK3568 BootROM
  -> Rockchip DDR blob
  -> board-isolated rk chainloader
  -> BL31 at EL3
       |-> optional OP-TEE BL32 at Secure EL1
       `-> EDK2 or U-Boot BL33 at Non-secure EL2
             -> EFI application or Linux
```

Neither option is implemented by the current chainloader. Its FIT policy
rejects BL32, constructs TF-A parameters with BL32 absent, and accepts only the
manifest-selected U-Boot as BL33. The normal firmware's `jump_to_payload()` is
also not the integration point for either component: it is the FUEFI EL2
payload path, not a complete TF-A secure-world handoff.

### EDK2 as an alternative BL33

An EDK2 port would be a new, explicitly selected chainloader backend, not a
change to the existing U-Boot variants. It would need its own board manifest,
firmware-volume artifacts, load and stack policy, memory map, and nonvolatile
variable-storage policy. At minimum, a useful port needs UART, PSCI through
BL31, a full board DTB, PCIe/NVMe Block I/O, FAT, EFI boot management, and
reset services. USB, MMC, SPI, and HDMI GOP remain separate board-driver work.

The community [Quartz64 UEFI project](https://github.com/jaredmcneill/quartz64_uefi)
is a useful RK3566/RK3568 reference and includes ROC-RK3566-PC, but an image or
GPIO policy from one board must never be inherited by YY3568, ROCK 3A, or
another RK356x board. The official
[TianoCore platform collection](https://github.com/tianocore/edk2-platforms/tree/master/Platform)
does not currently provide a generic RK3568 target.

U-Boot's EFI Loader already supports the repository's practical requirement of
starting `EFI/BOOT/BOOTAA64.EFI`. EDK2 is most useful when a native UEFI
environment, EFI Shell, persistent variables, or a future UEFI Secure Boot
policy is required; U-Boot remains the smaller path for NVMe Linux boot.

### OP-TEE as optional BL32

OP-TEE must run in the secure world and cannot be appended as a normal FUEFI
payload or substituted for BL33. A board port would require all of the
following:

- A manifest opt-in that pins the OP-TEE source or reviewed binary, hash,
  entry point, secure DRAM carveout, and shared-memory range.
- BL31 configured with a compatible OP-TEE dispatcher, plus TF-A parameters
  that describe BL32 and the normal-world BL33 independently.
- RK3568 security-controller and memory-firewall programming that protects
  secure RAM, with those ranges excluded from the chainloader, EDK2/U-Boot,
  DTB, and Linux memory maps.
- A Linux `/firmware/optee` DT node, `CONFIG_TEE` and `CONFIG_OPTEE`; userspace
  trusted applications additionally need the OP-TEE client and supplicant.
- Per-board tests for secure/non-secure entry state, SMC behavior, address
  overlap, reset, suspend, and failure recovery.

The official [OP-TEE Rockchip platform](https://github.com/OP-TEE/optee_os/tree/master/core/arch/arm/plat-rockchip)
does not currently include an RK3568 platform flavor. The rkbin collection has
an `rk3568_bl32_v2.16.bin`, but its implementation, source correspondence,
license, ABI, and memory policy must be established before it can be treated as
a supported OP-TEE input. Addresses from RK3399 or RK3588 OP-TEE ports must not
be reused on RK3568.

### Trust boundary and recommended order

FIT SHA-256 hashes detect corruption but do not authenticate firmware. Loading
OP-TEE therefore does not by itself establish a trusted device: an attacker
who can replace unauthenticated boot media can replace BL31, BL32, or BL33.
Authenticated BootROM loading, signed manifests or FITs, rollback protection,
and secure storage are distinct future features.

The preferred development order is:

1. Retain U-Boot as the default BL33.
2. Add and validate EDK2 as a board-scoped alternative BL33.
3. Add an independently selectable RK3568 OP-TEE port and secure-memory policy.
4. Add authenticated boot only when an actual security boundary is required.

Any EDK2 or OP-TEE implementation must preserve the existing board/variant
object namespaces and require explicit manifest policy. Enabling it for one
board must not change another board's U-Boot, address map, GPIOs, storage, or
release contents. TF-A defines the relevant
[BL31/BL32/BL33 model](https://trustedfirmware-a.readthedocs.io/en/latest/design/firmware-design.html)
and [secure-partition interfaces](https://trustedfirmware-a.readthedocs.io/en/latest/components/secure-partition-manager.html).

## Adding another board without collisions

A new board is unsupported until it provides a schema-validated manifest and
board overlay. The manifest must pin its U-Boot backend/repository/ref/commit,
BL31, artifact names, FIT staging, TF-A parameters, BL31 ranges, BL33 load and
stack boundary, boot targets, physical flash, pinmux, storage IDs, offsets,
capacity policy, and backup behavior.

Its automatic OS policy must map that board's NVMe devices, removable SD
slots, USB mass-storage class, and eMMC device in the required order. MMC
indices must be verified from that board's U-Boot aliases; an SDIO/Wi-Fi
controller must never be copied into the SD-card group.

The validator rejects unknown boards/backends, missing or extra policy fields,
path escapes, cross-board overlays/artifacts, mismatched names, and overlapping
memory ranges. Builds must use `build/chainload/<board>/`; copying another
board's DTS, GPIOs, addresses, storage IDs, or restore metadata is forbidden.
A port also needs CI matrix coverage, a deliberate release decision, memory-map
review, and hardware sign-off.

Hardware acceptance for each board covers USB and SD entry, eMMC boot with SPI
blank, SPI priority, eMMC fallback, source logging, storage probes, BL31/U-Boot,
PCIe/NVMe, and automatic OS discovery from NVMe, SD, USB, and eMMC in the
declared fallback order. Test both extlinux Linux and `BOOTAA64.EFI`, plus
invalid-FIT recovery, backup restore, and complete UART transcripts. Optional
ROCK 3A eMMC must abort safely when it is not populated.

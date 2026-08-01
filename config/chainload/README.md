# Chainloader board manifests

Chainloading is opt-in and board-scoped. A board is supported only when this
directory contains a manifest plus its own overlay. The manifest pins the
U-Boot and BL31 inputs, address policy, output names, and boot policy. No
fallback manifest exists: an unknown `BOARD` is rejected.

Schema 3 currently validates two independent policies:

| Board | Backend | BL33 load / stack | Automatic OS targets |
| --- | --- | --- | --- |
| `yy3568` | `vendor-fit` | `0x00a00000` / `0x00c00000` | `nvme0 nvme1 mmc1 usb0 mmc0` |
| `rock3a` | `mainline-fit` | `0x00800000` / `0x03f00000` | `nvme mmc1 usb mmc0` |

YY3568 imports board material from Armbian commit
`587b6f2c0a867859ca3f323f6008bee9e3ef1553` and was checked against the local
Youyeetoo `ArmBoardBringUp` reference. ROCK 3A uses the official mainline
`rock-3a-rk3568_defconfig` and DTS at U-Boot v2026.04; the same Armbian commit's
`rock-3a.conf` is its board-selection reference.

Run `python3 tools/chainload-manifest.py validate --all` before invoking a
board build. The validator closes the schema and rejects unknown backends,
missing or extra fields, repository path escapes, cross-board overlay/media
selection, mismatched names, artifact collisions, invalid formats/offsets, and
overlapping stage/FIT/BL33 ranges.

Every manifest declares `boot_policy.automatic_scan` with the fixed media
order `nvme`, `sd`, `usb`, `emmc` and a board-specific target list for each
medium. The validator rejects missing groups, reordering, duplicate devices,
and MMC/USB/NVMe names in the wrong group. On both current boards `mmc1` is the
removable SD slot and `mmc0` is eMMC. Their `mmc2` alias is onboard SDIO and is
intentionally absent from OS discovery. Backend spelling remains explicit:
the vendor distro framework uses `nvme0`/`usb0`, while mainline bootstd uses
the `nvme`/`usb` classes.

Adding a board requires all of the following:

1. A new manifest and board-specific U-Boot overlay.
2. A manifest-generated platform descriptor and isolated
   `build/chainload/<board>/` linker/object namespace.
3. A reviewed FIT staging area, BL31 ranges, BL33 load/stack bounds, and TF-A
   handoff protocol.
4. Explicit boot-media entries for each physical flash type: BootROM priority,
   format, detected-capacity policy, pinmux, rkdeveloptool storage ID, write
   offset, partition-preservation policy, backup scope, and artifact name.
5. A board-qualified U-Boot OS scan mapping for NVMe, removable SD, USB mass
   storage, and eMMC. Each `mmcN` must be checked against that board's aliases;
   SDIO devices must not be included.
6. Offline parser/address/media-policy tests, a chainloader CI matrix entry,
   release allowlist changes, and hardware validation of installation and
   restoration. Never reuse another board's addresses, GPIOs, storage IDs,
   artifacts, restore metadata, or DTS data by default.

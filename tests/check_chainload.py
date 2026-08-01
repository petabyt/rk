#!/usr/bin/env python3
"""Offline isolation/provenance checks and built RK356x chainload checks."""

from __future__ import annotations

import argparse
import copy
import hashlib
import importlib.util
import json
import os
import pathlib
import struct
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
PINNED_UBOOT = "39cd993e5d6296635438e84f4576b3a9bf76f86e"
PINNED_MAINLINE_UBOOT = "88dc2788777babfd6322fa655df549a019aa1e69"
PINNED_ARMBIAN = "587b6f2c0a867859ca3f323f6008bee9e3ef1553"
PINNED_RKBIN = "ecb4fcbe954edf38b3ae037d5de6d9f5bccf81f4"
PINNED_XROCK = "b90d3ba8f0a48320e3888701f7e66e0e4e038bbb"
PINNED_RKDEVELOPTOOL = "304f073752fd25c854e1bcf05d8e7f925b1f4e14"
BL31_HASH = "c81ac7e8e1fd727cf7f0db62a9aaea760bde2b270e34d98eb264a264b86df749"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def parse_load_segments(data: bytes) -> list[tuple[int, int, int]]:
    require(data[:4] == b"\x7fELF" and data[4] == 2 and data[5] == 1,
            "BL31 is not a little-endian ELF64 image")
    phoff = struct.unpack_from("<Q", data, 32)[0]
    phentsize, phnum = struct.unpack_from("<HH", data, 54)
    segments = []
    for index in range(phnum):
        offset = phoff + index * phentsize
        p_type, _, _, _, paddr, filesz, memsz, _ = struct.unpack_from(
            "<IIQQQQQQ", data, offset)
        if p_type == 1 and memsz:
            segments.append((paddr, filesz, memsz))
    return segments


def align(value: int, boundary: int) -> int:
    return (value + boundary - 1) & ~(boundary - 1)


def check_rkns_v2(raw: bytes, ddr: bytes, payload: bytes) -> None:
    """Validate the pinned U-Boot RKNS v2 rksd byte layout."""
    ddr_size = align(len(ddr), 2048)
    payload_size = align(len(payload), 2048)
    require(len(raw) == 2048 + ddr_size + payload_size,
            "RKNS v2 ID block has an unexpected size")
    require(len(raw) % 2048 == 0, "RKNS v2 ID block is not 2-KiB aligned")
    magic, reserved, size_images, boot_flag = struct.unpack_from("<IIII", raw, 0)
    require(magic == 0x534E4B52, "RKNS v2 magic mismatch")
    require(reserved == 0, "RKNS v2 reserved header word is nonzero")
    require(size_images == (2 << 16) | 384,
            "RKNS v2 header hash offset/image count mismatch")
    require(boot_flag == 1, "RKNS v2 must select SHA-256")
    require(raw[16:120] == bytes(104), "RKNS v2 reserved header bytes are nonzero")

    expected = ((4, ddr_size, ddr, 1),
                (4 + ddr_size // 512, payload_size, payload, 2))
    for index, (sector_offset, padded_size, source, counter) in enumerate(expected):
        entry_offset = 120 + index * 88
        size_offset, address, flag, actual_counter = struct.unpack_from(
            "<IIII", raw, entry_offset)
        require(size_offset == ((padded_size // 512) << 16) | sector_offset,
                f"RKNS v2 image {index} size/offset mismatch")
        require(address == 0xFFFFFFFF and flag == 0 and actual_counter == counter,
                f"RKNS v2 image {index} metadata mismatch")
        require(raw[entry_offset + 16:entry_offset + 24] == bytes(8),
                f"RKNS v2 image {index} reserved bytes are nonzero")
        padded = source + bytes(padded_size - len(source))
        require(raw[entry_offset + 24:entry_offset + 56] == hashlib.sha256(padded).digest(),
                f"RKNS v2 image {index} payload hash mismatch")
        require(raw[entry_offset + 56:entry_offset + 88] == bytes(32),
                f"RKNS v2 image {index} SHA-512 tail is nonzero")
        data_offset = sector_offset * 512
        require(raw[data_offset:data_offset + padded_size] == padded,
                f"RKNS v2 image {index} payload/padding mismatch")

    require(raw[120 + 2 * 88:0x600] == bytes(0x600 - (120 + 2 * 88)),
            "unused RKNS v2 entries/reserved area are nonzero")
    require(raw[0x600:0x620] == hashlib.sha256(raw[:0x600]).digest(),
            "RKNS v2 header hash mismatch")
    require(raw[0x620:0x800] == bytes(0x800 - 0x620),
            "RKNS v2 header signature padding is nonzero")


def check_media_images(board: str, manifest: dict[str, object]) -> None:
    ddr = (ROOT / "img" / "rk3568_ddr_1560MHz_v1.25.bin").read_bytes()
    artifacts = manifest["artifacts"]
    assert isinstance(artifacts, dict)
    payload = (ROOT / str(artifacts["binary"])).read_bytes()
    idblock = (ROOT / str(artifacts["idblock"])).read_bytes()
    spi = (ROOT / str(artifacts["spi_nor"])).read_bytes()
    sd = (ROOT / str(artifacts["image"])).read_bytes()
    check_rkns_v2(idblock, ddr, payload)
    require(sd[:64 * 512] == bytes(64 * 512),
            f"{board}: SD convenience image does not leave LBA 0-63 empty")
    require(sd[64 * 512:] == idblock,
            "SD convenience image does not contain the validated ID block at LBA 0x40")
    require(len(spi) == len(idblock) * 2,
            "rkspi image does not use the first-2-KiB-of-each-4-KiB layout")
    for offset in range(0, len(idblock), 2048):
        spi_offset = offset * 2
        require(spi[spi_offset:spi_offset + 2048] == idblock[offset:offset + 2048],
                f"rkspi data mismatch at raw offset 0x{offset:x}")
        require(spi[spi_offset + 2048:spi_offset + 4096] == bytes(2048),
                f"rkspi padding is nonzero at raw offset 0x{offset:x}")


def load_manifests() -> dict[str, dict[str, object]]:
    subprocess.run(
        [sys.executable, str(ROOT / "tools" / "chainload-manifest.py"),
         "validate", "--all"], check=True
    )
    paths = sorted((ROOT / "config" / "chainload").glob("*.json"))
    require([item.name for item in paths] == ["rock3a.json", "yy3568.json"],
            "chainloader board list changed without CI/release review")
    unknown = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "chainload-manifest.py"),
         "validate", "not-a-board"], text=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    require(unknown.returncode != 0 and "has no chainload manifest" in unknown.stderr,
            "unknown chainloader board was accepted")
    return {path.stem: json.loads(path.read_text(encoding="utf-8")) for path in paths}


def check_manifests(manifests: dict[str, dict[str, object]]) -> None:
    expected = {
        "yy3568": ("vendor-fit", PINNED_UBOOT, "next-dev-v2024.10",
                   "0x00a00000", "0x00c00000", {
                       "nvme": ["nvme0", "nvme1"], "sd": ["mmc1"],
                       "usb": ["usb0"], "emmc": ["mmc0"],
                   }),
        "rock3a": ("mainline-fit", PINNED_MAINLINE_UBOOT, "v2026.04",
                   "0x00800000", "0x03f00000", {
                       "nvme": ["nvme"], "sd": ["mmc1"],
                       "usb": ["usb"], "emmc": ["mmc0"],
                   }),
    }
    artifacts: set[str] = set()
    for board, values in expected.items():
        manifest = manifests[board]
        backend, commit, ref, load, stack, targets = values
        require(manifest["schema"] == 3 and manifest["board"] == board and
                manifest["soc"] == "rk3568" and manifest["platform"] == "rk3568",
                f"{board}: manifest identity/schema mismatch")
        uboot = manifest["uboot"]
        assert isinstance(uboot, dict)
        require(uboot["backend"] == backend and uboot["commit"] == commit and
                uboot["ref"] == ref and uboot["armbian_commit"] == PINNED_ARMBIAN,
                f"{board}: U-Boot/Armbian provenance changed")
        if board == "rock3a":
            require(uboot["armbian_path"] == "config/boards/rock-3a.conf",
                    "ROCK 3A Armbian board reference changed")
        bl31 = manifest["bl31"]
        assert isinstance(bl31, dict)
        require(bl31["rkbin_commit"] == PINNED_RKBIN and
                bl31["sha256"] == BL31_HASH, f"{board}: BL31 provenance changed")
        layout = manifest["layout"]
        assert isinstance(layout, dict)
        require(layout["stage_limit"] == "0x00040000" and
                layout["bl33_load"] == load and layout["bl33_stack"] == stack and
                layout["expected_bl31_segments"] == 6,
                f"{board}: memory policy changed without review")
        policy = manifest["boot_policy"]
        assert isinstance(policy, dict)
        scan = policy["automatic_scan"]
        assert isinstance(scan, dict)
        require(scan["order"] == ["nvme", "sd", "usb", "emmc"] and
                scan["targets"] == targets and
                policy["interactive_only"] == ["spi"] and
                policy["baud_rate"] == 1500000,
                f"{board}: automatic scan policy is not board-scoped")
        host_tools = manifest["host_tools"]
        assert isinstance(host_tools, dict)
        require(host_tools["xrock"]["commit"] == PINNED_XROCK and
                host_tools["rkdeveloptool"]["commit"] == PINNED_RKDEVELOPTOOL,
                f"{board}: host-tool compatibility pin changed")
        board_artifacts = manifest["artifacts"]
        assert isinstance(board_artifacts, dict)
        for artifact in board_artifacts.values():
            require(str(artifact) not in artifacts,
                    f"cross-board artifact collision: {artifact}")
            artifacts.add(str(artifact))


def check_manifest_rejections(manifests: dict[str, dict[str, object]]) -> None:
    spec = importlib.util.spec_from_file_location(
        "chainload_manifest_test", ROOT / "tools" / "chainload-manifest.py")
    require(spec is not None and spec.loader is not None,
            "cannot import chainload manifest validator")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    with tempfile.TemporaryDirectory(prefix="rk-chainload-manifest-") as temporary:
        manifest_dir = pathlib.Path(temporary)
        module.MANIFEST_DIR = manifest_dir

        def rejected(change, description: str) -> None:
            fixtures = copy.deepcopy(manifests)
            change(fixtures)
            for board, data in fixtures.items():
                (manifest_dir / f"{board}.json").write_text(
                    json.dumps(data), encoding="utf-8")
            try:
                module.validate_all()
            except module.ManifestError:
                return
            raise AssertionError(f"manifest validator accepted {description}")

        rejected(lambda items: items["rock3a"].update(board="yy3568"),
                 "a mismatched board name")
        rejected(lambda items: items["rock3a"]["uboot"].update(backend="unknown"),
                 "an unsupported U-Boot backend")
        rejected(lambda items: items["rock3a"]["uboot"].update(
            overlay="config/chainload/yy3568/overlay"),
            "another board's overlay")
        rejected(lambda items: items["rock3a"]["uboot"].update(
            config_fragment="../yy3568.config"), "a path escape")
        rejected(lambda items: items["rock3a"]["artifacts"].update(
            binary="uboot_yy3568.bin"), "a cross-board artifact")
        rejected(lambda items: items["rock3a"]["layout"].update(
            fit_stage_start="0x00800000"), "overlapping memory ranges")
        rejected(lambda items: items["rock3a"]["boot_media"]["emmc"].update(
            artifact="uboot_yy3568_idbloader.img"), "cross-board media")
        rejected(lambda items: items["rock3a"]["boot_media"]["spi-nor"].update(
            unknown_policy=True), "an unknown media policy field")
        rejected(lambda items: items["rock3a"]["boot_policy"]["automatic_scan"].update(
            order=["nvme", "usb", "sd", "emmc"]), "an incorrect OS scan order")
        rejected(lambda items: items["rock3a"]["boot_policy"]["automatic_scan"]
                 ["targets"].update(emmc=["mmc1"]),
                 "one MMC target assigned to both SD and eMMC")
        rejected(lambda items: items["rock3a"]["boot_policy"]["automatic_scan"]
                 ["targets"].update(sd=["usb0"]),
                 "a USB device assigned to the SD group")
        rejected(lambda items: items["rock3a"].pop("boot_policy"),
                 "a missing required policy")


def check_bl31() -> None:
    blob = (ROOT / "img" / "rk3568_bl31_v1.46.elf").read_bytes()
    require(len(blob) == 402376, "BL31 size mismatch")
    require(hashlib.sha256(blob).hexdigest() == BL31_HASH, "BL31 hash mismatch")
    ranges = ((0x40000, 0x200000), (0xFDCC0000, 0xFDCF0000))
    segments = parse_load_segments(blob)
    require(len(segments) == 6, "expected exactly six split BL31 load segments")
    for address, _, memory_size in segments:
        end = address + memory_size
        require(any(address >= start and end <= limit for start, limit in ranges),
                f"BL31 segment 0x{address:x}-0x{end:x} violates manifest policy")


def check_isolation() -> None:
    makefile = (ROOT / "Makefile").read_text(encoding="utf-8")
    normal_board = (ROOT / "src" / "rk356x" / "board.c").read_text(encoding="utf-8")
    loader = (ROOT / "src" / "chainload" / "loader.c").read_text(encoding="utf-8")
    linker = (ROOT / "Chainload-rk356x.ld").read_text(encoding="utf-8")
    builder = (ROOT / "tools" / "build-chainload-uboot.sh").read_text(encoding="utf-8")
    compat = (ROOT / "src" / "chainload" / "compat.c").read_text(encoding="utf-8")
    media_builder = (ROOT / "tools" / "build-chainload-media.sh").read_text(encoding="utf-8")
    flasher = (ROOT / "tools" / "flash-chainload.sh").read_text(encoding="utf-8")
    chainfit = (ROOT / "tools" / "chainfit.c").read_text(encoding="utf-8")
    require("build/chainload/$(1)/obj/" in makefile and
            "CHAINLOAD_BOARDS := yy3568 rock3a" in makefile,
            "chainloader objects lack generated board/variant namespaces")
    for symbol in ("YY3568_OBJ :=", "ROCK3A_OBJ :="):
        line = next(item for item in makefile.splitlines() if item.startswith(symbol))
        require("chain" not in line.lower(), f"normal {symbol} firmware inherited chainloader objects")
    require("chainload" not in normal_board and "FIT" not in normal_board,
            "normal RK356x entry gained FIT auto-detection")
    common_list = makefile[makefile.index("CHAINLOAD_SRC :="):makefile.index("CHAINLOAD_CFLAGS :=")]
    chainfit_rule = makefile[makefile.index("tools/chainfit.out:"):
                             makefile.index("\nchainload:")]
    for excluded in ("firmware.c", "ohci.c", "hid_keyboard.c", "hdmi.c", "vop2.c", "demo/"):
        require(excluded not in common_list, f"dedicated chainloader includes {excluded}")
    require("ASSERT(_end_of_image < 0x00040000" in linker,
            "chainloader link limit is missing")
    require("first BL31 load address\");" not in linker,
            "chainloader ASSERT uses GNU ld-incompatible trailing semicolon")
    require("size_t strnlen(const char *s, size_t maxlen)" in compat,
            "chainloader libfdt compatibility layer lacks strnlen")
    require("-D_POSIX_C_SOURCE=200809L" in makefile and
            "src/chainload/loader.c src/chainload/compat.c" in makefile,
            "host chainloader tests do not exercise the strnlen compatibility layer")
    require("-D_POSIX_C_SOURCE=200809L" in chainfit_rule and
            "src/chainload/compat.c" in chainfit_rule,
            "chainfit host tool lacks the strnlen compatibility contract")
    require(loader.index("dcache_clean") < loader.index("chain_jump_bl31"),
            "loaded ranges are not cleaned before the BL31 branch")
    require("fdt_check_header" in loader and "fit_stage_start" in loader,
            "FIT is not staged before loading")
    require("vendor-fit)" in builder and "mainline-fit)" in builder and
            "u-boot.bin tools u-boot.its u-boot.dtb" in builder and
            'CFLAGS="$vendor_flags" KCFLAGS="$vendor_flags"' in builder and
            "fix-u-boot-itb-dependency-on-u-boot-dtb.patch" in
            (ROOT / "config" / "chainload" / "yy3568" / "overlay" /
             "README.rk-chainload").read_text(encoding="utf-8") and
            "dumpimage -l u-boot.itb" in builder and "zero-byte image" in builder and
            '[[ -x "$snapshot/tools/mkimage" ]]' in builder and
            "all tools/mkimage" not in builder,
            "U-Boot builder does not isolate vendor/mainline FIT backends")
    require('"$mkimage" -n "$soc" -T rksd' in media_builder and
            '"$mkimage" -n "$soc" -T rkspi' in media_builder,
            "chainload media is not generated by pinned U-Boot mkimage")
    require("makeboot.out" not in media_builder,
            "chainload media accidentally uses the normal firmware packer")
    for marker in ("cs 1", "cs 9", "check-partition-overlap.py", "cmp \"$artifact\"",
                   "complete-spi-nor.bin", "emmc-lba0-63.bin"):
        require(marker in (ROOT / "docs" / "rk356x" / "chainloading.md").read_text(encoding="utf-8")
                or marker in flasher,
                f"guarded media installer lacks contract marker: {marker}")
    platform = (ROOT / "src" / "chainload" / "rk3568.c").read_text()
    require("0xfdcc0010" in platform and 'return "source=spi-nor"' in platform and
            'return "source=emmc"' in platform and 'return "source=sd"' in platform and
            'return "source=usb"' in platform,
            "RK3568 chainloader lacks BootROM-source decoding")
    require("CHAIN_EXPECTED_BL33_ENTRY" in platform and
            "CHAIN_BL31_RANGE_INITIALIZER" in platform,
            "chainloader platform does not consume the generated board descriptor")
    require("BootROM download marker" in platform,
            "validation recovery does not document MaskROM reset")
    require("deprecated alias" in makefile and "usb-chainload" in makefile,
            "ambiguous RK3568 USB aliases were not replaced safely")
    require("chainfit-args" in makefile and "yy3568" not in chainfit and
            "rock3a" not in chainfit,
            "host FIT validation bypasses manifest-derived board policy")


def check_overlays() -> None:
    base = ROOT / "config" / "chainload" / "yy3568" / "overlay"
    config = (base / "configs" / "yy3568-rk3568_defconfig").read_text()
    dts = (base / "arch" / "arm" / "dts" / "rk3568-yy3568.dts").read_text()
    header = (base / "include" / "configs" / "yy3568.h").read_text()
    board_makefile = (base / "board" / "rockchip" / "yy3568" /
                      "Makefile").read_text()
    board_source = (base / "board" / "rockchip" / "yy3568" /
                    "yy3568.c").read_text()
    board_kconfig = (base / "board" / "rockchip" / "evb_rk3568" /
                     "Kconfig").read_text()
    for setting in (
        "CONFIG_BOOTDELAY=3", "CONFIG_BAUDRATE=1500000", "CONFIG_NVME=y",
        "CONFIG_CMD_NVME=y", "CONFIG_CMD_EXT4=y", "CONFIG_CMD_FAT=y",
        "CONFIG_CMD_SF=y", "CONFIG_CMD_MMC=y", "CONFIG_CMD_USB=y",
        "CONFIG_USB_STORAGE=y", "CONFIG_EFI_LOADER=y", "CONFIG_TARGET_YY3568=y",
        "# CONFIG_DM_CHARGE_DISPLAY is not set",
        "# CONFIG_CHARGE_ANIMATION is not set",
    ):
        require(setting in config, f"YY3568 U-Boot config lacks {setting}")
    require("setenv boot_targets nvme0 nvme1 mmc1 usb0 mmc0; run distro_bootcmd" in header,
            "YY3568 board header does not enforce its automatic scan order")
    require('default "yy3568" if TARGET_YY3568' in board_kconfig and
            "obj-y += yy3568.o" in board_makefile and
            '#include "../evb_rk3568/evb_rk3568.c"' in board_source,
            "YY3568 target lacks its isolated vendor board shim")
    for alias in ("mmc0 = &sdhci;", "mmc1 = &sdmmc0;", "mmc2 = &sdmmc2;"):
        require(alias in dts, f"YY3568 U-Boot DTS lacks board alias {alias}")
    for node in ("&pcie2x1", "&pcie3x2", "vcc3v3_pcie", "reset-gpios"):
        require(node in dts, f"YY3568 U-Boot DTS lacks {node}")
    for marker in ("&sfc", "&fspi_pins", "&spi_nor", "&spi_nand", "&sdhci",
                   "&emmc_bus8", "bus-width = <8>"):
        require(marker in dts, f"YY3568 storage DTS lacks {marker}")
    require(dts[dts.index("&spi_nand"):].split("};", 1)[0].find('status = "disabled"') >= 0,
            "YY3568 SPI-NAND child is not disabled")
    require(dts[dts.index("&spi_nor"):].split("};", 1)[0].find('status = "okay"') >= 0,
            "YY3568 JEDEC SPI-NOR child is not enabled")
    require("youyeetoo,yy3568" in dts and "rockchip,rk3568" in dts,
            "YY3568 U-Boot DTS identity mismatch")

    rock = ROOT / "config" / "chainload" / "rock3a" / "overlay"
    fragment = (rock / "configs" / "rock3a-chainload.config").read_text()
    config_header = (rock / "include" / "configs" /
                     "evb_rk3568.h").read_text()
    uboot_dtsi = (rock / "arch" / "arm" / "dts" /
                  "rk3568-rock-3a-u-boot.dtsi").read_text()
    source_readme = (rock / "README.rk-chainload").read_text()
    for setting in (
        "CONFIG_BOOTDELAY=3", "CONFIG_BAUDRATE=1500000",
        "CONFIG_BOOTSTD_DEFAULTS=y",
        "CONFIG_BOOTMETH_EXTLINUX=y", "CONFIG_BOOTMETH_SCRIPT=y",
        "CONFIG_CMD_NVME=y", "CONFIG_CMD_EXT4=y", "CONFIG_CMD_FAT=y",
        "CONFIG_CMD_SF=y", "CONFIG_CMD_MMC=y", "CONFIG_CMD_USB=y",
        "CONFIG_USB_STORAGE=y", "CONFIG_NVME_PCI=y", "CONFIG_EFI_LOADER=y",
    ):
        require(setting in fragment, f"ROCK 3A config fragment lacks {setting}")
    require('#define BOOT_TARGETS "nvme mmc1 usb mmc0"' in config_header and
            "#include <configs/rk3568_common.h>" in config_header and
            "ROCKCHIP_DEVICE_SETTINGS" in config_header,
            "ROCK 3A board header does not enforce its automatic scan order")
    require("/delete-property/ fit,external-offset" in uboot_dtsi and
            '#include "rk356x-u-boot.dtsi"' in uboot_dtsi,
            "ROCK 3A does not preserve upstream DTS while forcing inline FIT data")
    require("rock-3a-rk3568_defconfig" in source_readme and
            "configs/rock3a-chainload.config" in source_readme and
            "include/configs/evb_rk3568.h" in source_readme,
            "ROCK 3A corresponding source lacks reproducible build instructions")


def check_built(board: str) -> None:
    manifests = load_manifests()
    require(board in manifests, "an unsupported board reached chainload checks")
    manifest = manifests[board]
    artifacts = manifest["artifacts"]
    uboot_policy = manifest["uboot"]
    assert isinstance(artifacts, dict) and isinstance(uboot_policy, dict)
    stage = ROOT / "build" / "chainload" / board / "stage.bin"
    require(stage.is_file() and stage.stat().st_size < 0x40000,
            f"{board}: chainloader stage exceeds its dedicated low-memory limit")
    fit = ROOT / str(artifacts["fit"])
    binary = ROOT / str(artifacts["binary"])
    require(fit.is_file() and binary.is_file(), "chainload outputs are incomplete")
    require(binary.read_bytes() == stage.read_bytes() + fit.read_bytes(),
            f"{board}: combined image is not stage + pinned FIT")
    fit_args = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "chainload-manifest.py"),
         "chainfit-args", board], check=True, text=True,
        stdout=subprocess.PIPE,
    ).stdout.split()
    subprocess.run([str(ROOT / "tools" / "chainfit.out"), board, str(fit),
                    *fit_args], check=True)
    if os.environ.get("UBOOT_ITB"):
        return
    source = ROOT / "build" / "chainload" / board / "source"
    config = (source / ".config").read_text(encoding="utf-8")
    common_settings = (
        "CONFIG_BOOTDELAY=3", "CONFIG_BAUDRATE=1500000", "CONFIG_CMD_NVME=y",
        "CONFIG_CMD_MMC=y", "CONFIG_CMD_USB=y", "CONFIG_USB_STORAGE=y",
        "CONFIG_EFI_LOADER=y", "CONFIG_CMD_BOOTEFI=y",
    )
    backend_settings = {
        "vendor-fit": ("CONFIG_TARGET_YY3568=y", "CONFIG_NVME=y", "CONFIG_CMD_SF=y"),
        "mainline-fit": ("CONFIG_TEXT_BASE=0x00800000", "CONFIG_NVME_PCI=y",
                         'CONFIG_SYS_CONFIG_NAME="evb_rk3568"',
                         "CONFIG_BOOTMETH_EXTLINUX=y", "CONFIG_BOOTMETH_EFILOADER=y"),
    }
    for setting in common_settings + backend_settings[str(uboot_policy["backend"])]:
        require(setting in config, f"{board}: built U-Boot config lacks {setting}")
    uboot = (source / "u-boot.bin").read_bytes()
    if board == "yy3568":
        require(b"setenv boot_targets nvme0 nvme1 mmc1 usb0 mmc0; run distro_bootcmd" in uboot,
                "YY3568 U-Boot does not contain its automatic scan command")
    else:
        require(b"boot_targets=nvme mmc1 usb mmc0" in uboot,
                "ROCK 3A U-Boot does not contain its automatic scan environment")
        require(b"boot_targets=mmc1 mmc0 nvme scsi usb pxe dhcp spi" not in uboot,
                "ROCK 3A U-Boot retained the broad RK3568 default boot targets")
    compiled_dts = subprocess.run(
        ["dtc", "-I", "dtb", "-O", "dts", str(source / "u-boot.dtb")],
        check=True, stdout=subprocess.PIPE,
    ).stdout.decode("utf-8")
    identity = "youyeetoo,yy3568" if board == "yy3568" else "radxa,rock3a"
    for marker in (identity, "pcie@fe260000", "pcie@fe280000",
                   "reset-gpios", "jedec,spi-nor"):
        require(marker in compiled_dts, f"{board}: built U-Boot DTB lacks {marker}")
    spi_name = "sfc@fe300000" if board == "yy3568" else "spi@fe300000"
    spi_node = f"{spi_name} {{"
    require(spi_node in compiled_dts,
            f"{board}: built U-Boot DTB lacks its SPI-NOR controller")
    spi = compiled_dts[compiled_dts.index(spi_node):].split("};", 1)[0]
    require('status = "okay"' in spi,
            f"{board}: built U-Boot DTB does not enable SPI NOR")
    emmc_name = "sdhci@fe310000" if board == "yy3568" else "mmc@fe310000"
    emmc_node = f"{emmc_name} {{"
    require(emmc_node in compiled_dts,
            f"{board}: built U-Boot DTB lacks its enabled eMMC controller")
    emmc = compiled_dts[compiled_dts.index(emmc_node):].split("};", 1)[0]
    require('status = "okay"' in emmc,
            f"{board}: built U-Boot DTB does not enable eMMC")
    require((ROOT / str(artifacts["source"])).is_file(),
            f"{board}: corresponding U-Boot source archive was not generated")
    check_media_images(board, manifest)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--offline", action="store_true")
    parser.add_argument("--board")
    args = parser.parse_args()
    manifests = load_manifests()
    check_manifests(manifests)
    check_manifest_rejections(manifests)
    check_bl31()
    check_isolation()
    check_overlays()
    if args.offline:
        for board in manifests:
            stage = ROOT / "build" / "chainload" / board / "stage.bin"
            require(stage.is_file() and stage.stat().st_size < 0x40000,
                    f"{board}: offline chainloader stage was not built or is oversized")
    if args.board:
        check_built(args.board)
    print("chainloader checks passed")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.CalledProcessError) as error:
        print(f"chainload check failed: {error}", file=sys.stderr)
        raise SystemExit(1)

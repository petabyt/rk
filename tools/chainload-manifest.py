#!/usr/bin/env python3
"""Validate and query board-scoped chainloader manifests."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import sys
from typing import Any

ROOT = pathlib.Path(__file__).resolve().parents[1]
MANIFEST_DIR = ROOT / "config" / "chainload"
BOARD_RE = re.compile(r"^[a-z0-9][a-z0-9_-]*$")
BACKENDS = {"vendor-fit", "mainline-fit"}
ARTIFACT_KEYS = ("fit", "binary", "image", "idblock", "spi_nor", "source")
BOOT_ORDER = ["spi-nor", "spi-nand", "nand", "emmc", "sd", "usb"]
AUTOMATIC_MEDIA_ORDER = ["nvme", "sd", "usb", "emmc"]


class ManifestError(ValueError):
    pass


def fail(message: str) -> None:
    raise ManifestError(message)


def integer(value: Any, field: str) -> int:
    if isinstance(value, bool):
        fail(f"{field} must be an integer")
    try:
        parsed = int(value, 0) if isinstance(value, str) else int(value)
    except (TypeError, ValueError):
        fail(f"{field} must be an integer")
    if parsed < 0:
        fail(f"{field} may not be negative")
    return parsed


def relative_path(value: Any, field: str) -> pathlib.PurePosixPath:
    if not isinstance(value, str) or not value:
        fail(f"{field} must be a non-empty path")
    path = pathlib.PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or "\\" in value:
        fail(f"{field} must stay inside the repository")
    return path


def repository_entry(value: Any, field: str, directory: bool = False) -> pathlib.Path:
    relative = relative_path(value, field)
    path = ROOT.joinpath(*relative.parts)
    valid = path.is_dir() if directory else path.is_file()
    if not valid or path.is_symlink():
        fail(f"{field} does not name a safe {'directory' if directory else 'file'}")
    return path


def mapping(value: Any, field: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        fail(f"{field} must be an object")
    return value


def nonempty_string(value: Any, field: str) -> str:
    if not isinstance(value, str) or not value:
        fail(f"{field} must be a non-empty string")
    return value


def require_keys(value: dict[str, Any], keys: set[str], field: str,
                 optional: set[str] | None = None) -> None:
    optional = optional or set()
    missing = keys - value.keys()
    extra = value.keys() - keys - optional
    if missing:
        fail(f"{field} is missing: {', '.join(sorted(missing))}")
    if extra:
        fail(f"{field} has unknown fields: {', '.join(sorted(extra))}")


def load_raw(board: str) -> dict[str, Any]:
    if not BOARD_RE.fullmatch(board):
        fail(f"invalid board identifier: {board}")
    path = MANIFEST_DIR / f"{board}.json"
    if not path.is_file():
        fail(f"board '{board}' has no chainload manifest")
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        fail(f"cannot read {path.relative_to(ROOT)}: {error}")
    if not isinstance(data, dict):
        fail(f"{path.name} must contain an object")
    return data


def validate(board: str) -> dict[str, Any]:
    data = load_raw(board)
    require_keys(data, {
        "schema", "board", "identity", "soc", "platform", "uboot", "bl31",
        "layout", "artifacts", "boot_media", "host_tools", "boot_policy",
    }, board)
    if data["schema"] != 3:
        fail(f"{board}: unsupported manifest schema")
    if data["board"] != board:
        fail(f"{board}: manifest identity does not match its filename")
    if data["soc"] != "rk3568" or data["platform"] != "rk3568":
        fail(f"{board}: unsupported SoC/platform")
    nonempty_string(data["identity"], f"{board}.identity")

    uboot = mapping(data["uboot"], f"{board}.uboot")
    require_keys(uboot, {
        "backend", "repository", "ref", "commit", "defconfig", "overlay",
        "armbian_repository", "armbian_commit",
    }, f"{board}.uboot", {"config_fragment", "armbian_path"})
    if uboot["backend"] not in BACKENDS:
        fail(f"{board}: unsupported U-Boot backend")
    for field in ("repository", "ref", "defconfig", "armbian_repository"):
        nonempty_string(uboot[field], f"{board}.uboot.{field}")
    if not re.fullmatch(r"[0-9a-f]{40}", str(uboot["commit"])):
        fail(f"{board}: U-Boot commit must be a full SHA-1")
    if not re.fullmatch(r"[0-9a-f]{40}", str(uboot["armbian_commit"])):
        fail(f"{board}: Armbian commit must be a full SHA-1")
    overlay = relative_path(uboot["overlay"], f"{board}.uboot.overlay")
    expected_overlay = pathlib.PurePosixPath("config", "chainload", board, "overlay")
    if overlay != expected_overlay:
        fail(f"{board}: overlay must be isolated under {expected_overlay}")
    overlay_path = repository_entry(uboot["overlay"], f"{board}.uboot.overlay",
                                    directory=True)
    if any(path.is_symlink() for path in overlay_path.rglob("*")):
        fail(f"{board}: overlay may not contain symbolic links")
    if uboot["backend"] == "mainline-fit":
        fragment = relative_path(uboot.get("config_fragment"),
                                 f"{board}.uboot.config_fragment")
        if fragment.parts[:len(overlay.parts)] != overlay.parts:
            fail(f"{board}: config fragment escapes its board overlay")
        repository_entry(uboot["config_fragment"],
                         f"{board}.uboot.config_fragment")
        relative_path(uboot.get("armbian_path"), f"{board}.uboot.armbian_path")
    elif "config_fragment" in uboot:
        fail(f"{board}: vendor-fit does not accept a config fragment")

    bl31 = mapping(data["bl31"], f"{board}.bl31")
    require_keys(bl31, {
        "path", "rkbin_repository", "rkbin_commit", "rkbin_path", "size", "sha256",
    }, f"{board}.bl31")
    bl31_path = repository_entry(bl31["path"], f"{board}.bl31.path")
    bl31_size = integer(bl31["size"], f"{board}.bl31.size")
    if bl31_size == 0:
        fail(f"{board}: BL31 may not be empty")
    if not re.fullmatch(r"[0-9a-f]{64}", str(bl31["sha256"])):
        fail(f"{board}: BL31 SHA-256 is invalid")
    if bl31_path.stat().st_size != bl31_size or \
            hashlib.sha256(bl31_path.read_bytes()).hexdigest() != bl31["sha256"]:
        fail(f"{board}: BL31 file does not match its provenance")

    layout = mapping(data["layout"], f"{board}.layout")
    require_keys(layout, {
        "stage_limit", "fit_stage_start", "fit_stage_end", "bl31_params",
        "bl31_entry", "bl31_ranges", "expected_bl31_segments", "bl33_load",
        "bl33_stack", "handoff_protocol",
    }, f"{board}.layout")
    stage_limit = integer(layout["stage_limit"], f"{board}.layout.stage_limit")
    fit_start = integer(layout["fit_stage_start"], f"{board}.layout.fit_stage_start")
    fit_end = integer(layout["fit_stage_end"], f"{board}.layout.fit_stage_end")
    params = integer(layout["bl31_params"], f"{board}.layout.bl31_params")
    bl31_entry = integer(layout["bl31_entry"], f"{board}.layout.bl31_entry")
    bl33_load = integer(layout["bl33_load"], f"{board}.layout.bl33_load")
    bl33_stack = integer(layout["bl33_stack"], f"{board}.layout.bl33_stack")
    segments = integer(layout["expected_bl31_segments"],
                       f"{board}.layout.expected_bl31_segments")
    if stage_limit != 0x40000 or not (fit_start < fit_end) or not (bl33_load < bl33_stack):
        fail(f"{board}: invalid stage/FIT/BL33 bounds")
    if max(bl33_load, fit_start) < min(bl33_stack, fit_end):
        fail(f"{board}: BL33 range overlaps the FIT staging arena")
    if params >= 0x200000 or segments == 0:
        fail(f"{board}: invalid TF-A parameter or segment policy")
    if layout["handoff_protocol"] != "tf-a-v1-bl33-aarch64-el2":
        fail(f"{board}: unsupported handoff protocol")
    ranges = layout["bl31_ranges"]
    if not isinstance(ranges, list) or not ranges:
        fail(f"{board}: BL31 ranges must be a non-empty list")
    parsed_ranges: list[tuple[int, int]] = []
    for index, item in enumerate(ranges):
        if not isinstance(item, list) or len(item) != 2:
            fail(f"{board}: BL31 range {index} must contain start and end")
        start = integer(item[0], f"{board}.layout.bl31_ranges[{index}][0]")
        end = integer(item[1], f"{board}.layout.bl31_ranges[{index}][1]")
        if start >= end:
            fail(f"{board}: BL31 range {index} is empty")
        parsed_ranges.append((start, end))
    if not any(start <= bl31_entry < end for start, end in parsed_ranges):
        fail(f"{board}: BL31 entry is outside its permitted ranges")
    for index, first in enumerate(parsed_ranges):
        for second in parsed_ranges[index + 1:]:
            if max(first[0], second[0]) < min(first[1], second[1]):
                fail(f"{board}: permitted BL31 ranges overlap")
    named_regions = (
        ("stage", 0, stage_limit),
        ("FIT staging", fit_start, fit_end),
        ("BL33", bl33_load, bl33_stack),
    )
    for index, first in enumerate(named_regions):
        for second in named_regions[index + 1:]:
            if max(first[1], second[1]) < min(first[2], second[2]):
                fail(f"{board}: {first[0]} and {second[0]} ranges overlap")
    if any(start <= params < end for _, start, end in named_regions):
        fail(f"{board}: TF-A parameters overlap a loaded/staging range")

    artifacts = mapping(data["artifacts"], f"{board}.artifacts")
    require_keys(artifacts, set(ARTIFACT_KEYS), f"{board}.artifacts")
    expected_artifacts = {
        "fit": f"{board}-u-boot.itb",
        "binary": f"uboot_{board}.bin",
        "image": f"uboot_{board}.img",
        "idblock": f"uboot_{board}_idbloader.img",
        "spi_nor": f"uboot_{board}_spi.img",
        "source": f"{board}-u-boot-source.tar.xz",
    }
    if artifacts != expected_artifacts:
        fail(f"{board}: artifact names must be board-qualified")
    for key, value in artifacts.items():
        path = relative_path(value, f"{board}.artifacts.{key}")
        if len(path.parts) != 1:
            fail(f"{board}: artifacts must be emitted at the repository root")

    media = mapping(data["boot_media"], f"{board}.boot_media")
    require_keys(media, {"bootrom_order", "ddr", "usbplug", "sd", "emmc", "spi-nor"},
                 f"{board}.boot_media")
    if media["bootrom_order"] != BOOT_ORDER:
        fail(f"{board}: RK3568 BootROM order changed")
    repository_entry(media["ddr"], f"{board}.boot_media.ddr")
    repository_entry(media["usbplug"], f"{board}.boot_media.usbplug")
    expected_media_artifacts = {
        "sd": artifacts["image"], "emmc": artifacts["idblock"],
        "spi-nor": artifacts["spi_nor"],
    }
    sd = mapping(media["sd"], f"{board}.boot_media.sd")
    emmc = mapping(media["emmc"], f"{board}.boot_media.emmc")
    spi = mapping(media["spi-nor"], f"{board}.boot_media.spi-nor")
    require_keys(sd, {"format", "idblock_lba", "artifact"},
                 f"{board}.boot_media.sd")
    require_keys(emmc, {"format", "storage_id", "write_lba", "capacity_policy",
                        "preserve_partition_table", "required_pinctrl", "artifact"},
                 f"{board}.boot_media.emmc")
    require_keys(spi, {"format", "storage_id", "write_lba", "capacity_policy",
                       "required_pinctrl", "artifact"},
                 f"{board}.boot_media.spi-nor")
    if sd["format"] != "rksd" or emmc["format"] != "rksd" or spi["format"] != "rkspi":
        fail(f"{board}: unsupported Rockchip media format")
    if integer(sd["idblock_lba"], f"{board}.boot_media.sd.idblock_lba") != 64 or \
            integer(emmc["write_lba"], f"{board}.boot_media.emmc.write_lba") != 64 or \
            integer(spi["write_lba"], f"{board}.boot_media.spi-nor.write_lba") != 0:
        fail(f"{board}: BootROM media offsets are invalid")
    if emmc["preserve_partition_table"] is not True:
        fail(f"{board}: eMMC policy must preserve the partition table")
    if emmc["capacity_policy"] != "detected-size-must-cover-image" or \
            spi["capacity_policy"] != "detected-size-must-cover-image":
        fail(f"{board}: unsupported detected-capacity policy")
    for name, entry in (("sd", sd), ("emmc", emmc), ("spi-nor", spi)):
        if entry["artifact"] != expected_media_artifacts[name]:
            fail(f"{board}: {name} selects another board's artifact")
    for name, entry in (("emmc", emmc), ("spi-nor", spi)):
        integer(entry["storage_id"], f"{board}.boot_media.{name}.storage_id")
        nonempty_string(entry["required_pinctrl"],
                        f"{board}.boot_media.{name}.required_pinctrl")

    host_tools = mapping(data["host_tools"], f"{board}.host_tools")
    require_keys(host_tools, {"xrock", "rkdeveloptool"}, f"{board}.host_tools")
    for name in ("xrock", "rkdeveloptool"):
        tool = mapping(host_tools[name], f"{board}.host_tools.{name}")
        require_keys(tool, {"repository", "commit"}, f"{board}.host_tools.{name}")
        nonempty_string(tool["repository"], f"{board}.host_tools.{name}.repository")
        if not re.fullmatch(r"[0-9a-f]{40}", str(tool["commit"])):
            fail(f"{board}: {name} commit must be a full SHA-1")

    policy = mapping(data["boot_policy"], f"{board}.boot_policy")
    require_keys(policy, {
        "automatic_scan", "boot_delay_seconds", "baud_rate", "interactive_only",
        "formats",
    }, f"{board}.boot_policy")
    scan = mapping(policy["automatic_scan"], f"{board}.boot_policy.automatic_scan")
    require_keys(scan, {"order", "targets"},
                 f"{board}.boot_policy.automatic_scan")
    if scan["order"] != AUTOMATIC_MEDIA_ORDER:
        fail(f"{board}: automatic scan order must be NVMe, SD, USB, then eMMC")
    targets = mapping(scan["targets"],
                      f"{board}.boot_policy.automatic_scan.targets")
    require_keys(targets, set(AUTOMATIC_MEDIA_ORDER),
                 f"{board}.boot_policy.automatic_scan.targets")
    target_patterns = {
        "nvme": r"nvme(?:[0-9]+)?",
        "sd": r"mmc[0-9]+",
        "usb": r"usb(?:[0-9]+)?",
        "emmc": r"mmc[0-9]+",
    }
    flattened: list[str] = []
    for medium in AUTOMATIC_MEDIA_ORDER:
        entries = targets[medium]
        if not isinstance(entries, list) or not entries or any(
                not isinstance(target, str) or
                not re.fullmatch(target_patterns[medium], target)
                for target in entries):
            fail(f"{board}: invalid automatic {medium} targets")
        flattened.extend(entries)
    if len(flattened) != len(set(flattened)):
        fail(f"{board}: automatic boot targets overlap across media")
    if not isinstance(policy["interactive_only"], list) or \
            not all(isinstance(item, str) and item for item in policy["interactive_only"]):
        fail(f"{board}: interactive-only commands must be a string list")
    if set(policy["interactive_only"]) & {"nvme", "mmc", "usb"}:
        fail(f"{board}: automatically scanned media cannot be interactive-only")
    if not isinstance(policy["formats"], list) or \
            not all(isinstance(item, str) and item for item in policy["formats"]):
        fail(f"{board}: boot formats must be a string list")
    integer(policy["boot_delay_seconds"], f"{board}.boot_policy.boot_delay_seconds")
    if integer(policy["baud_rate"], f"{board}.boot_policy.baud_rate") != 1500000:
        fail(f"{board}: UART must remain at 1.5 Mbaud")
    return data


def manifests() -> list[str]:
    return [path.stem for path in sorted(MANIFEST_DIR.glob("*.json"))]


def validate_all() -> dict[str, dict[str, Any]]:
    boards = manifests()
    if not boards:
        fail("no chainloader manifests found")
    result = {board: validate(board) for board in boards}
    owners: dict[str, str] = {}
    for board, data in result.items():
        for artifact in data["artifacts"].values():
            previous = owners.setdefault(artifact, board)
            if previous != board:
                fail(f"artifact collision: {artifact} belongs to {previous} and {board}")
    return result


def lookup(data: Any, dotted: str) -> Any:
    value = data
    for component in dotted.split("."):
        if not isinstance(value, dict) or component not in value:
            fail(f"manifest field does not exist: {dotted}")
        value = value[component]
    return value


def c_hex(value: Any) -> str:
    return f"0x{integer(value, 'generated value'):x}UL"


def generate_header(board: str, output: pathlib.Path) -> None:
    data = validate(board)
    layout = data["layout"]
    ranges = layout["bl31_ranges"]
    range_lines = (" " + chr(92) + "\n").join(
        f"\t{{ {c_hex(item[0])}, {c_hex(item[1])} }}," for item in ranges
    )
    content = f"""/* Generated from config/chainload/{board}.json; do not edit. */
#ifndef CHAINLOAD_BOARD_CONFIG_H
#define CHAINLOAD_BOARD_CONFIG_H

#define CHAIN_BOARD_NAME {json.dumps(data['identity'])}
#define CHAIN_SOC_NAME "Rockchip RK3568"
#define CHAIN_FIT_STAGE_START {c_hex(layout['fit_stage_start'])}
#define CHAIN_FIT_STAGE_END {c_hex(layout['fit_stage_end'])}
#define CHAIN_PARAMS_ADDR {c_hex(layout['bl31_params'])}
#define CHAIN_EXPECTED_BL31_ENTRY {c_hex(layout['bl31_entry'])}
#define CHAIN_EXPECTED_BL33_ENTRY {c_hex(layout['bl33_load'])}
#define CHAIN_BL33_LIMIT {c_hex(layout['bl33_stack'])}
#define CHAIN_EXPECTED_BL31_SEGMENTS {integer(layout['expected_bl31_segments'], 'segments')}
#define CHAIN_BL31_RANGE_COUNT {len(ranges)}
#define CHAIN_BL31_RANGE_INITIALIZER \\
{range_lines}

#endif
"""
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_suffix(output.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8", newline="\n")
    temporary.replace(output)


def main() -> int:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("list")
    validate_parser = subparsers.add_parser("validate")
    validate_parser.add_argument("board", nargs="?")
    validate_parser.add_argument("--all", action="store_true")
    get_parser = subparsers.add_parser("get")
    get_parser.add_argument("board")
    get_parser.add_argument("field")
    artifacts_parser = subparsers.add_parser("artifacts")
    artifacts_parser.add_argument("board")
    fit_args_parser = subparsers.add_parser("chainfit-args")
    fit_args_parser.add_argument("board")
    header_parser = subparsers.add_parser("generate-header")
    header_parser.add_argument("board")
    header_parser.add_argument("output", type=pathlib.Path)
    args = parser.parse_args()

    if args.command == "list":
        validate_all()
        print(" ".join(manifests()))
    elif args.command == "validate":
        if args.all:
            validate_all()
        elif args.board:
            validate(args.board)
        else:
            fail("validate requires BOARD or --all")
    elif args.command == "get":
        value = lookup(validate(args.board), args.field)
        if isinstance(value, (dict, list)):
            print(json.dumps(value, separators=(",", ":")))
        elif isinstance(value, bool):
            print("true" if value else "false")
        else:
            print(value)
    elif args.command == "artifacts":
        data = validate(args.board)
        print(" ".join(str(data["artifacts"][key]) for key in ARTIFACT_KEYS))
    elif args.command == "chainfit-args":
        layout = validate(args.board)["layout"]
        values = [
            layout["fit_stage_start"], layout["fit_stage_end"],
            layout["bl31_params"], layout["bl31_entry"], layout["bl33_load"],
            layout["bl33_stack"], layout["expected_bl31_segments"],
        ]
        for start, end in layout["bl31_ranges"]:
            values.extend((start, end))
        print(" ".join(str(value) for value in values))
    elif args.command == "generate-header":
        generate_header(args.board, args.output)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ManifestError as error:
        print(f"chainload manifest error: {error}", file=sys.stderr)
        raise SystemExit(2)

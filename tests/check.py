#!/usr/bin/env python3
"""Artifact and source-contract checks for the RK356x bring-up."""

from __future__ import annotations

import hashlib
import pathlib
import re
import struct
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]

BLOBS = {
    "rk3566_ddr_1056MHz_v1.25.bin": (59392, "c2a1b37673bf03ed338bc39efbe942136459cb3621dad09351144d744d78db26"),
    "rk3568_ddr_1560MHz_v1.25.bin": (59392, "ab1d9b822a256b6ef4b3aa54b911c4d1e0faaebc882403c7a6b3efc3e69e07fc"),
    "rk356x_usbplug_v1.17.bin": (98708, "4038b7857b840f539760decc0daf1601b8ff61cc17798101e93b11128a7f333e"),
    "rk3568_bl31_v1.46.elf": (402376, "c81ac7e8e1fd727cf7f0db62a9aaea760bde2b270e34d98eb264a264b86df749"),
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def check_docs() -> None:
    docs = ROOT / "docs"
    required = {
        "rk356x/index.md",
        "rk356x/bare-metal.md",
        "rk356x/chainloading.md",
        "rk356x/boards/roc3566.md",
        "rk356x/boards/yy3568.md",
        "rk356x/boards/rock3a.md",
    }
    markdown = {
        path.relative_to(docs).as_posix()
        for path in docs.rglob("*.md")
    }
    require(required <= markdown, "RK356x documentation hierarchy is incomplete")
    require(not (docs / "rk356x.md").exists() and
            not (docs / "chainloading.md").exists(),
            "legacy root-level RK356x documentation was reintroduced")

    for path in docs.rglob("*.md"):
        source = path.read_text(encoding="utf-8")
        for match in re.finditer(r"\]\(([^)]+)\)", source):
            target = match.group(1).split("#", 1)[0]
            if not target or target.startswith(("http://", "https://", "mailto:", "/")):
                continue
            require((path.parent / target).is_file(),
                    f"{path.relative_to(ROOT)}: broken local link: {target}")

    mkdocs = (ROOT / "mkdocs.yml").read_text(encoding="utf-8")
    nav = set(re.findall(r"^\s+-\s+[^:]+:\s+([A-Za-z0-9_./-]+\.md)\s*$",
                         mkdocs, re.MULTILINE))
    require(nav == markdown,
            f"MkDocs navigation mismatch: missing={sorted(markdown - nav)}, "
            f"extra={sorted(nav - markdown)}")

    boards = {
        "roc3566": ("Firefly ROC-RK3566-PC", "firefly,roc-rk3566-pc"),
        "yy3568": ("Youyeetoo YY3568", "youyeetoo,yy3568"),
        "rock3a": ("Radxa ROCK 3A", "radxa,rock3a"),
    }
    packager = (ROOT / "tools" / "release-dist.sh").read_text(encoding="utf-8")
    for board, markers in boards.items():
        board_path = docs / "rk356x" / "boards" / f"{board}.md"
        board_doc = board_path.read_text(encoding="utf-8")
        require(all(marker in board_doc for marker in (board, *markers)),
                f"{board}: board documentation lacks identity markers")
        require(f"docs/rk356x/boards/{board}.md BOARD.md" in packager,
                f"{board}: release does not select its board documentation")
    require("docs/rk356x/chainloading.md chainloading.md" in packager,
            "release uses a stale RK356x chainloading path")


def check_blobs() -> None:
    for name, (size, digest) in BLOBS.items():
        data = (ROOT / "img" / name).read_bytes()
        require(len(data) == size, f"{name}: expected {size} bytes")
        require(hashlib.sha256(data).hexdigest() == digest, f"{name}: SHA-256 mismatch")


def image_entry(header: bytes, index: int) -> tuple[int, int]:
    entry_size = 88
    offset = 0x78 + index * entry_size
    return struct.unpack_from("<HH", header, offset)


def check_image(image_name: str, ddr_name: str, os_name: str) -> None:
    image = (ROOT / image_name).read_bytes()
    ddr = (ROOT / "img" / ddr_name).read_bytes()
    os_image = (ROOT / os_name).read_bytes()
    header = image[0x8000 : 0x8800]
    signature, _, hash_offset, count, boot_flag = struct.unpack_from("<IIHHI", header)
    require(signature == 0x534E4B52, f"{image_name}: not RKNS v2")
    require(hash_offset == 0x180 and count == 2 and boot_flag == 0x4000,
            f"{image_name}: invalid RKNS header")
    init_size = len(ddr) // 512 + 1
    os_size = len(os_image) // 512 + 1
    os_size = (os_size // 8 + 1) * 8
    require(image_entry(header, 0) == (4, init_size), f"{image_name}: DDR entry mismatch")
    require(image_entry(header, 1) == (init_size + 4, os_size),
            f"{image_name}: payload entry mismatch")
    require(image[0x8800 : 0x8800 + len(ddr)] == ddr, f"{image_name}: DDR data mismatch")
    os_offset = 0x8800 + init_size * 512
    require(image[os_offset : os_offset + len(os_image)] == os_image,
            f"{image_name}: OS data mismatch")


def check_payload(board: str) -> None:
    firmware = (ROOT / f"{board}.bin").read_bytes()
    demo = (ROOT / "demo.bin").read_bytes()
    combined = (ROOT / f"demo_{board}.bin").read_bytes()
    require(combined == firmware + demo, f"demo_{board}.bin is not firmware + payload")
    require(struct.unpack_from("<I", demo, 0x28)[0] == 0x08008135,
            "demo payload magic is missing")
    flags, image_size = struct.unpack_from("<II", demo, 0x30)
    relocation = struct.unpack_from("<Q", demo, 0x38)[0]
    require(flags & 1, "demo payload must request relocation")
    require(relocation == 0x00A00000, "demo payload relocation changed")
    require(image_size == len(demo), "demo payload header size is not exact")
    require(relocation + image_size <= 0x07FF0000,
            "demo payload overlaps the RK356x stack")
    require(len(firmware) < 0x200000, f"{board}: firmware overlaps reserved first 2 MiB")


def check_dts() -> None:
    expected = {
        "roc3566.dts": ("Firefly ROC-RK3566-PC", "firefly,roc-rk3566-pc", "rockchip,rk3566"),
        "yy3568.dts": ("Youyeetoo YY3568", "youyeetoo,yy3568", "rockchip,rk3568"),
        "rock3a.dts": ("Radxa ROCK 3A", "radxa,rock3a", "rockchip,rk3568"),
    }
    for name, strings in expected.items():
        path = ROOT / "src" / "rk356x" / name
        source = path.read_text(encoding="utf-8")
        for value in strings:
            require(value in source, f"{name}: missing {value}")
        cpp = subprocess.run(
            ["cpp", "-nostdinc", "-undef", "-x", "assembler-with-cpp", str(path)],
            check=True, stdout=subprocess.PIPE,
        )
        dtb = subprocess.run(
            ["dtc", "-I", "dts", "-O", "dtb", "-o", "-"],
            input=cpp.stdout, check=True, stdout=subprocess.PIPE,
        )
        compiled = subprocess.run(
            ["dtc", "-I", "dtb", "-O", "dts"], input=dtb.stdout,
            check=True, stdout=subprocess.PIPE,
        ).stdout.decode("utf-8")
        for value in strings:
            require(value in compiled, f"{name}: compiled DTB lost {value}")
        for unit in ("serial@fe660000", "hdmi@fe0a0000"):
            start = compiled.find(unit + " {")
            end = compiled.find("};", start)
            require(start >= 0 and end > start and
                    'status = "okay"' in compiled[start:end],
                    f"{name}: enabled {unit} node is missing")
        enabled_hosts = ["usb@fd840000"]
        if name in ("yy3568.dts", "rock3a.dts"):
            enabled_hosts.append("usb@fd8c0000")
        for unit in enabled_hosts:
            start = compiled.find(unit + " {")
            end = compiled.find("};", start)
            require(start >= 0 and end > start and
                    'status = "okay"' in compiled[start:end],
                    f"{name}: enabled {unit} node is missing")
        if name == "rock3a.dts":
            for wiring in ("gpios = <&gpio0 15 0>",
                           "gpio = <&gpio0 6 0>",
                           "gpio = <&gpio0 29 0>"):
                require(wiring in source, f"ROCK 3A wiring changed: {wiring}")


def check_source_contracts() -> None:
    makefile = (ROOT / "Makefile").read_text(encoding="utf-8")
    rock = (ROOT / "tools" / "rock.c").read_text(encoding="utf-8")
    io = (ROOT / "src" / "rk356x" / "io.c").read_text(encoding="utf-8")
    ohci = (ROOT / "src" / "ohci.c").read_text(encoding="utf-8")
    hdmi = (ROOT / "src" / "rk356x" / "hdmi.c").read_text(encoding="utf-8")
    board = (ROOT / "src" / "rk356x" / "board.c").read_text(encoding="utf-8")
    for target in ("usb3566", "usb3568", "usb:", "usb-chainload",
                   "maskrom3566", "maskrom3568"):
        marker = target if target.endswith(":") else f"{target}:"
        require(marker in makefile, f"Makefile lacks {target.rstrip(':')}")
    require(makefile.count("--v2 --ddr img/rk356") >= 4, "RK356x images must use RKNS v2")
    require("--rc4-off" in makefile, "xrock flow must explicitly disable RC4")
    require("case 0x350a" in rock and 'soc = "RK356x"' in rock,
            "rock.out lacks RK356x PID handling")
    require("Multiple Rockchip MaskROM devices" in rock and "Unsupported Rockchip PID" in rock,
            "rock.out lacks unambiguous selection diagnostics")
    for address in ("0x07ff0000", "0x08000000", "0x08400000", "0x10000000", "0x12000000"):
        require(address.lower() in (ROOT / "src" / "rk356x" / "rk356x.h").read_text().lower(),
                f"layout constant {address} is missing")
    require("address >= RK356X_MMIO_START ? 0 : 3" in io,
            "MMIO page-table device mapping is missing")
    require("enumerate_step" in ohci and "control_status" in ohci and
            "retry_after" in ohci,
            "OHCI attach/retry must use the nonblocking enumeration state machine")
    require("period < c->interval" in ohci and "periodic_stop" in ohci,
            "OHCI periodic interval/detach handling is missing")
    require("if (!(hdmi_read(0x3004) & 2))" in hdmi and
            "rk356x_video.active = 1" in hdmi,
            "HDMI headless/active state handling is missing")
    require("if (rk356x_video.active)" in board and
            "screens->type = FU_SCREEN_XRGB8888" in board,
            "FUEFI screen metadata/headless behavior is missing")

    stride_4k = (3840 * 4 + 63) & ~63
    require(stride_4k * 2160 <= 0x12000000 - 0x10000000,
            "4K XRGB8888 framebuffer exceeds its arena")


def main() -> int:
    check_docs()
    check_blobs()
    check_source_contracts()
    check_dts()
    for board, ddr in (("roc3566", "rk3566_ddr_1056MHz_v1.25.bin"),
                       ("yy3568", "rk3568_ddr_1560MHz_v1.25.bin"),
                       ("rock3a", "rk3568_ddr_1560MHz_v1.25.bin")):
        check_payload(board)
        check_image(f"{board}.img", ddr, f"{board}.bin")
        check_image(f"demo_{board}.img", ddr, f"demo_{board}.bin")
    print("RK356x artifact checks passed")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.CalledProcessError) as error:
        print(f"check failed: {error}", file=sys.stderr)
        raise SystemExit(1)

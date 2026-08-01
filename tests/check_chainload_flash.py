#!/usr/bin/env python3
"""Exercise the board-scoped RK356x installer against file-backed mock devices."""

from __future__ import annotations

import json
import os
import pathlib
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib

ROOT = pathlib.Path(__file__).resolve().parents[1]
FLASHER = ROOT / "tools" / "flash-chainload.sh"
PARTITION_CHECK = ROOT / "tools" / "check-partition-overlap.py"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def bash_path(path: pathlib.Path) -> str:
    """Return an absolute path accepted by both POSIX and Git Bash."""
    resolved = path.resolve().as_posix()
    if os.name == "nt" and len(resolved) >= 3 and resolved[1:3] == ":/":
        return f"/{resolved[0].lower()}{resolved[2:]}"
    return resolved


def host_path(path: pathlib.Path) -> str:
    return path.resolve().as_posix()


def run(command: list[str], env: dict[str, str] | None = None,
        check: bool = False) -> subprocess.CompletedProcess[str]:
    complete_env = os.environ.copy()
    if env:
        complete_env.update(env)
    result = subprocess.run(command, cwd=ROOT, env=complete_env, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if check and result.returncode:
        raise AssertionError(
            f"command failed ({result.returncode}): {' '.join(command)}\n"
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}"
        )
    return result


def write_mock(path: pathlib.Path) -> None:
    path.write_text(r'''#!/usr/bin/env python3
import os, pathlib, sys

state = pathlib.Path(os.environ["MOCK_STATE"])
state.mkdir(parents=True, exist_ok=True)
with (state / "commands.log").open("a", encoding="utf-8") as log:
    log.write(" ".join(sys.argv[1:]) + "\n")
args = sys.argv[1:]
if args == ["-h"]:
    print("ChangeStorage: cs [storage]")
elif args == ["ld"]:
    count = int(os.environ.get("MOCK_DEVICES", "1"))
    mode = os.environ.get("MOCK_MODE", "Loader")
    pid = os.environ.get("MOCK_PID", "350a")
    for index in range(count):
        print(f"DevNo={index + 1}\tVid=0x2207,Pid=0x{pid},LocationID={index + 1}\tMode={mode}")
elif args and args[0] == "cs":
    if os.environ.get("MOCK_CS_FAIL") == "1":
        print("Change Storage failed", file=sys.stderr)
        raise SystemExit(1)
    (state / "selected").write_text(args[1], encoding="ascii")
    print("Change Storage OK.")
elif args == ["rfi"]:
    selected = (state / "selected").read_text(encoding="ascii")
    media = pathlib.Path(os.environ["MOCK_EMMC" if selected == "1" else "MOCK_SPI"])
    sectors = int(os.environ.get("MOCK_CAPACITY", str(media.stat().st_size // 512)))
    print("Flash Info:")
    print(f"\tFlash Size: {sectors} Sectors")
elif args == ["rid"]:
    print(os.environ.get("MOCK_FLASH_ID", "Flash ID: EF 40 18 00 00"))
elif args and args[0] in ("rl", "wl"):
    selected = (state / "selected").read_text(encoding="ascii")
    media = pathlib.Path(os.environ["MOCK_EMMC" if selected == "1" else "MOCK_SPI"])
    data = bytearray(media.read_bytes())
    start = int(args[1]) * 512
    if args[0] == "rl":
        count = int(args[2]) * 512
        if start + count > len(data):
            raise SystemExit(1)
        output = bytearray(data[start:start + count])
        if os.environ.get("MOCK_SHORT_READ") == "1" and not (state / "wrote").exists():
            output = output[:-512]
        if (os.environ.get("MOCK_BAD_READBACK") == "1" and
                (state / "wrote").exists() and
                pathlib.Path(args[3]).name in ("installed-readback.bin", "restore-readback.bin")):
            output[0] ^= 0xff
        pathlib.Path(args[3]).write_bytes(output)
        print("Read LBA OK.")
    else:
        source = pathlib.Path(args[2]).read_bytes()
        if start + len(source) > len(data):
            raise SystemExit(1)
        data[start:start + len(source)] = source
        media.write_bytes(data)
        (state / "wrote").write_text("1", encoding="ascii")
        print("Write LBA OK.")
elif args == ["rd"]:
    (state / "reset").write_text("1", encoding="ascii")
    print("Reset Device OK.")
else:
    print(f"unsupported mock command: {args}", file=sys.stderr)
    raise SystemExit(2)
''', encoding="utf-8", newline="\n")
    path.chmod(0o755)


def fixture(parent: pathlib.Path) -> tuple[pathlib.Path, dict[str, str]]:
    repo = parent / "fixture"
    (repo / "config" / "chainload").mkdir(parents=True)
    (repo / "img").mkdir()
    (repo / "tools").mkdir()
    (repo / "img" / "ddr.bin").write_bytes(b"DDR")
    (repo / "img" / "usbplug.bin").write_bytes(b"USB")
    idblock = bytes((index * 29 + 7) & 0xff for index in range(1024))
    spi = bytes((index * 13 + 3) & 0xff for index in range(2048))
    (repo / "uboot_yy3568_idbloader.img").write_bytes(idblock)
    (repo / "uboot_yy3568_spi.img").write_bytes(spi)
    manifest = {
        "board": "yy3568",
        "boot_media": {
            "ddr": "img/ddr.bin",
            "usbplug": "img/usbplug.bin",
            "emmc": {"artifact": "uboot_yy3568_idbloader.img",
                     "storage_id": 1, "write_lba": 64},
            "spi-nor": {"artifact": "uboot_yy3568_spi.img",
                        "storage_id": 9, "write_lba": 0},
        },
    }
    (repo / "config" / "chainload" / "yy3568.json").write_text(
        json.dumps(manifest), encoding="utf-8")
    shutil.copy2(PARTITION_CHECK, repo / "tools" / PARTITION_CHECK.name)

    mock = parent / "rkdeveloptool"
    xrock = parent / "xrock"
    write_mock(mock)
    xrock.write_text("#!/usr/bin/env sh\necho xrock mock\n", encoding="utf-8", newline="\n")
    xrock.chmod(0o755)
    state = parent / "state"
    emmc = parent / "emmc.bin"
    spi_media = parent / "spi.bin"
    emmc.write_bytes(bytes(256 * 512))
    spi_media.write_bytes(bytes([0xff]) * (64 * 512))
    env = {
        "CHAINLOAD_REPO": bash_path(repo),
        "XROCK": bash_path(xrock),
        "RKDEVELOPTOOL": bash_path(mock),
        "MOCK_STATE": host_path(state),
        "MOCK_EMMC": host_path(emmc),
        "MOCK_SPI": host_path(spi_media),
    }
    return repo, env


def flash(env: dict[str, str], media: str, backup: pathlib.Path,
          confirmation: str | None = None) -> subprocess.CompletedProcess[str]:
    backup.mkdir(parents=True, exist_ok=True)
    return run(["bash", str(FLASHER), "flash", "yy3568", media,
                host_path(backup), confirmation or f"yy3568:{media}"], env)


def make_gpt(first: int, last: int) -> bytes:
    data = bytearray(64 * 512)
    data[446 + 4] = 0xEE
    struct.pack_into("<II", data, 446 + 8, 1, 255)
    data[510:512] = b"\x55\xaa"
    entries = bytearray(4 * 128)
    entries[:16] = bytes(range(1, 17))
    entries[16:32] = bytes(range(17, 33))
    struct.pack_into("<QQ", entries, 32, first, last)
    data[2 * 512:2 * 512 + len(entries)] = entries
    header = bytearray(512)
    header[:8] = b"EFI PART"
    struct.pack_into("<I", header, 8, 0x00010000)
    struct.pack_into("<I", header, 12, 92)
    struct.pack_into("<QQQQ", header, 24, 1, 255, 34, 222)
    header[56:72] = bytes(range(33, 49))
    struct.pack_into("<QIII", header, 72, 2, 4, 128,
                     zlib.crc32(entries) & 0xffffffff)
    struct.pack_into("<I", header, 16, zlib.crc32(header[:92]) & 0xffffffff)
    data[512:1024] = header
    return bytes(data)


def check_partition_tables(parent: pathlib.Path) -> None:
    safe = parent / "safe-gpt.bin"
    overlap = parent / "overlap-gpt.bin"
    safe.write_bytes(make_gpt(128, 160))
    overlap.write_bytes(make_gpt(64, 100))
    require(run(["python3", str(PARTITION_CHECK), str(safe), "64", "2"]).returncode == 0,
            "safe GPT layout was rejected")
    result = run(["python3", str(PARTITION_CHECK), str(overlap), "64", "2"])
    require(result.returncode != 0 and "GPT partition 1" in result.stderr,
            "overlapping GPT layout was accepted")

    mbr = bytearray(64 * 512)
    mbr[446 + 4] = 0x83
    struct.pack_into("<II", mbr, 446 + 8, 64, 4)
    mbr[510:512] = b"\x55\xaa"
    mbr_path = parent / "overlap-mbr.bin"
    mbr_path.write_bytes(mbr)
    result = run(["python3", str(PARTITION_CHECK), str(mbr_path), "64", "2"])
    require(result.returncode != 0 and "MBR partition 1" in result.stderr,
            "overlapping MBR layout was accepted")


def check_emmc_install_restore(parent: pathlib.Path) -> None:
    repo, env = fixture(parent)
    backup = parent / "backup"
    emmc = parent / "emmc.bin"
    before = emmc.read_bytes()
    result = flash(env, "emmc", backup)
    require(result.returncode == 0, f"mock eMMC flash failed: {result.stderr}")
    payload = (repo / "uboot_yy3568_idbloader.img").read_bytes()
    after = emmc.read_bytes()
    require(after[64 * 512:64 * 512 + len(payload)] == payload,
            "eMMC installer wrote the wrong range")
    require(after[:64 * 512] == before[:64 * 512],
            "eMMC installer changed LBA 0-63")
    require((backup / "emmc-lba0-63.bin").stat().st_size == 64 * 512,
            "eMMC metadata backup is incomplete")
    require((backup / "previous-idblock-region.bin").read_bytes() == bytes(len(payload)),
            "eMMC destination backup is incomplete")
    log = parent / "state" / "commands.log"
    require("cs 1\n" in log.read_text(encoding="utf-8"),
            "eMMC storage ID 1 was not selected")
    require((parent / "state" / "reset").is_file(),
            "successful install did not reset the board")

    result = run(["bash", str(FLASHER), "restore", host_path(backup),
                  "restore:yy3568:emmc"], env)
    require(result.returncode == 0, f"mock eMMC restore failed: {result.stderr}")
    restored = emmc.read_bytes()
    require(restored[64 * 512:64 * 512 + len(payload)] == bytes(len(payload)),
            "eMMC restore did not restore the destination range")


def check_spi_install(parent: pathlib.Path) -> None:
    repo, env = fixture(parent)
    # Exercise the release-bundle layout as well as the source-tree layout.
    (repo / "chainload").mkdir()
    (repo / "loaders").mkdir()
    (repo / "install").mkdir()
    shutil.move(repo / "config" / "chainload" / "yy3568.json",
                repo / "chainload" / "MANIFEST.json")
    shutil.move(repo / "uboot_yy3568_idbloader.img", repo / "chainload")
    shutil.move(repo / "uboot_yy3568_spi.img", repo / "chainload")
    shutil.move(repo / "img" / "ddr.bin", repo / "loaders")
    shutil.move(repo / "img" / "usbplug.bin", repo / "loaders")
    shutil.move(repo / "tools" / "check-partition-overlap.py", repo / "install")
    shutil.rmtree(repo / "config")
    shutil.rmtree(repo / "img")
    shutil.rmtree(repo / "tools")
    backup = parent / "backup"
    spi = parent / "spi.bin"
    before = spi.read_bytes()
    result = flash(env, "spi-nor", backup)
    require(result.returncode == 0, f"mock SPI-NOR flash failed: {result.stderr}")
    payload = (repo / "chainload" / "uboot_yy3568_spi.img").read_bytes()
    after = spi.read_bytes()
    require(after[:len(payload)] == payload and after[len(payload):] == before[len(payload):],
            "SPI-NOR installer changed bytes outside the image range")
    require((backup / "complete-spi-nor.bin").read_bytes() == before,
            "SPI-NOR full-device backup is incomplete")
    log = (parent / "state" / "commands.log").read_text(encoding="utf-8")
    require("cs 9\n" in log and "rid\n" in log and not any(
        line.startswith("ef") for line in log.splitlines()),
        "SPI-NOR selection/JEDEC/no-erase contract failed")


def check_failures(parent: pathlib.Path) -> None:
    cases = parent / "failures"
    cases.mkdir()

    _, env = fixture(cases / "confirmation")
    result = flash(env, "emmc", cases / "confirmation" / "backup", "wrong")
    require(result.returncode != 0 and "CONFIRM" in result.stderr,
            "malformed confirmation was accepted")
    require(not (cases / "confirmation" / "state" / "wrote").exists(),
            "confirmation failure touched storage")

    _, env = fixture(cases / "devices")
    env["MOCK_DEVICES"] = "2"
    result = flash(env, "emmc", cases / "devices" / "backup")
    require(result.returncode != 0 and "exactly one" in result.stderr,
            "multiple Rockchip devices were accepted")
    require(not (cases / "devices" / "state" / "wrote").exists(),
            "multiple-device failure touched storage")

    _, env = fixture(cases / "pid")
    env["MOCK_PID"] = "330c"
    result = flash(env, "emmc", cases / "pid" / "backup")
    require(result.returncode != 0 and "PID 0x350a" in result.stderr,
            "unsupported Rockchip PID was accepted")

    _, env = fixture(cases / "storage")
    env["MOCK_CS_FAIL"] = "1"
    result = flash(env, "emmc", cases / "storage" / "backup")
    require(result.returncode != 0 and
            not (cases / "storage" / "state" / "wrote").exists(),
            "storage-selection failure did not abort before writing")

    _, env = fixture(cases / "capacity")
    env["MOCK_CAPACITY"] = "2"
    result = flash(env, "spi-nor", cases / "capacity" / "backup")
    require(result.returncode != 0 and "capacity" in result.stderr,
            "insufficient SPI-NOR capacity was accepted")
    require(not (cases / "capacity" / "state" / "wrote").exists(),
            "capacity failure touched storage")

    _, env = fixture(cases / "jedec")
    env["MOCK_FLASH_ID"] = "Flash ID: 00 00 00 00 00"
    result = flash(env, "spi-nor", cases / "jedec" / "backup")
    require(result.returncode != 0 and "JEDEC manufacturer" in result.stderr,
            "invalid SPI-NOR JEDEC response was accepted")
    require(not (cases / "jedec" / "state" / "wrote").exists(),
            "JEDEC failure touched storage")

    _, env = fixture(cases / "short-backup")
    env["MOCK_SHORT_READ"] = "1"
    result = flash(env, "emmc", cases / "short-backup" / "backup")
    require(result.returncode != 0 and "backup has an unexpected size" in result.stderr,
            "short eMMC backup was accepted")
    require(not (cases / "short-backup" / "state" / "wrote").exists(),
            "short-backup failure touched storage")

    _, env = fixture(cases / "overlap")
    emmc = cases / "overlap" / "emmc.bin"
    data = bytearray(emmc.read_bytes())
    data[446 + 4] = 0x83
    struct.pack_into("<II", data, 446 + 8, 64, 4)
    data[510:512] = b"\x55\xaa"
    emmc.write_bytes(data)
    result = flash(env, "emmc", cases / "overlap" / "backup")
    require(result.returncode != 0 and "overlaps" in result.stderr,
            "overlapping eMMC partition was accepted")
    require(not (cases / "overlap" / "state" / "wrote").exists(),
            "partition-overlap failure touched storage")

    _, env = fixture(cases / "readback")
    env["MOCK_BAD_READBACK"] = "1"
    backup = cases / "readback" / "backup"
    result = flash(env, "emmc", backup)
    state = cases / "readback" / "state"
    require(result.returncode != 0 and "restore-chainload" in result.stderr,
            "readback mismatch was not reported with recovery guidance")
    require((backup / "backup.json").is_file() and not (state / "reset").exists(),
            "readback mismatch reset the board or omitted its backup")

    _, env = fixture(cases / "nonempty")
    backup = cases / "nonempty" / "backup"
    backup.mkdir()
    (backup / "keep.txt").write_text("keep\n", encoding="utf-8")
    result = flash(env, "emmc", backup)
    require(result.returncode != 0 and "must be empty" in result.stderr,
            "non-empty backup destination was accepted")
    require(not (cases / "nonempty" / "state" / "wrote").exists(),
            "non-empty-backup failure touched storage")

    _, env = fixture(cases / "missing-tool")
    env["RKDEVELOPTOOL"] = bash_path(cases / "missing-tool" / "not-installed")
    result = flash(env, "emmc", cases / "missing-tool" / "backup")
    require(result.returncode != 0 and "required tool not found" in result.stderr,
            "missing host tool was not rejected")

    _, env = fixture(cases / "missing")
    missing = cases / "missing" / "does-not-exist"
    result = run(["bash", str(FLASHER), "restore", host_path(missing),
                  "restore:yy3568:emmc"], env)
    require(result.returncode != 0 and "backup.json is missing" in result.stderr,
            "restore accepted a missing backup")

    repo, env = fixture(cases / "manifest-mismatch")
    backup = cases / "manifest-mismatch" / "backup"
    result = flash(env, "emmc", backup)
    require(result.returncode == 0, "manifest-mismatch fixture failed to install")
    manifest_path = repo / "config" / "chainload" / "yy3568.json"
    manifest_path.write_text(manifest_path.read_text(encoding="utf-8") + "\n",
                             encoding="utf-8")
    result = run(["bash", str(FLASHER), "restore", host_path(backup),
                  "restore:yy3568:emmc"], env)
    require(result.returncode != 0 and "different board manifest" in result.stderr,
            "restore accepted a backup from a different manifest revision")

    _, env = fixture(cases / "restore-capacity")
    backup = cases / "restore-capacity" / "backup"
    result = flash(env, "emmc", backup)
    require(result.returncode == 0, "restore-capacity fixture failed to install")
    env["MOCK_CAPACITY"] = "128"
    result = run(["bash", str(FLASHER), "restore", host_path(backup),
                  "restore:yy3568:emmc"], env)
    require(result.returncode != 0 and "capacity differs" in result.stderr,
            "restore accepted a different-capacity device")

    _, env = fixture(cases / "restore-range")
    backup = cases / "restore-range" / "backup"
    result = flash(env, "emmc", backup)
    require(result.returncode == 0, "restore-range fixture failed to install")
    metadata_path = backup / "backup.json"
    metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
    metadata["installed_sectors"] += 1
    metadata_path.write_text(json.dumps(metadata), encoding="utf-8")
    result = run(["bash", str(FLASHER), "restore", host_path(backup),
                  "restore:yy3568:emmc"], env)
    require(result.returncode != 0 and "saved range differs" in result.stderr,
            "restore accepted a backup for a different saved range")

    _, env = fixture(cases / "backup-schema")
    backup = cases / "backup-schema" / "backup"
    result = flash(env, "emmc", backup)
    require(result.returncode == 0, "backup-schema fixture failed to install")
    metadata_path = backup / "backup.json"
    metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
    metadata["schema"] = 1
    metadata_path.write_text(json.dumps(metadata), encoding="utf-8")
    result = run(["bash", str(FLASHER), "restore", host_path(backup),
                  "restore:yy3568:emmc"], env)
    require(result.returncode != 0 and "metadata schema" in result.stderr,
            "restore accepted unsupported backup metadata")

    repo, env = fixture(cases / "cross-board")
    backup = cases / "cross-board" / "backup"
    result = flash(env, "emmc", backup)
    require(result.returncode == 0, "cross-board fixture failed to install")
    yy_manifest = repo / "config" / "chainload" / "yy3568.json"
    rock_manifest = json.loads(yy_manifest.read_text(encoding="utf-8"))
    rock_manifest["board"] = "rock3a"
    (repo / "config" / "chainload" / "rock3a.json").write_text(
        json.dumps(rock_manifest), encoding="utf-8")
    yy_manifest.unlink()
    result = run(["bash", str(FLASHER), "restore", host_path(backup),
                  "restore:yy3568:emmc"], env)
    require(result.returncode != 0 and "has no chainload manifest" in result.stderr,
            "restore accepted a YY3568 backup from a ROCK 3A target bundle")


def main() -> int:
    test_root = ROOT / "build" / "host-tests"
    test_root.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="rk-chainload-flash-", dir=test_root) as temporary:
        parent = pathlib.Path(temporary)
        check_partition_tables(parent)
        check_emmc_install_restore(parent / "emmc")
        check_spi_install(parent / "spi")
        check_failures(parent)
    print("guarded chainload flashing checks passed")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.SubprocessError) as error:
        print(f"chainload flashing check failed: {error}", file=sys.stderr)
        raise SystemExit(1)

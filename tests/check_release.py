#!/usr/bin/env python3
"""Validate the deterministic GitHub release distribution contract."""

from __future__ import annotations

import hashlib
import io
import os
import pathlib
import shutil
import subprocess
import sys
import tarfile
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
PACKAGER = ROOT / "tools" / "release-dist.sh"
VERSION = "v1.2.3"

PACKAGES = {
    "pinebook-pro": {
        "BOARD.md",
        "BUILD-INFO.txt",
        "LICENSE",
        "README.md",
        "SHA256SUMS",
        "firmware/demo_pinebook.bin",
        "firmware/pinebook.bin",
        "images/demo_pinebook.img",
        "images/pinebook.img",
        "loaders/pinebook-ddr.bin",
        "loaders/pinebook-poc-ddr.bin",
    },
    "genbook": {
        "BOARD.md",
        "BUILD-INFO.txt",
        "LICENSE",
        "README.md",
        "SHA256SUMS",
        "firmware/demo_genbook.bin",
        "firmware/genbook.bin",
        "images/genbook.img",
        "images/genbook_demo.img",
        "loaders/genbook-ddr.bin",
    },
    "roc3566": {
        "BOARD.md",
        "BUILD-INFO.txt",
        "LICENSE",
        "README.md",
        "SHA256SUMS",
        "firmware/demo_roc3566.bin",
        "firmware/roc3566.bin",
        "images/demo_roc3566.img",
        "images/roc3566.img",
        "licenses/ROCKCHIP-BINARY-LICENSE",
        "loaders/rk3566_ddr_1056MHz_v1.25.bin",
        "loaders/rk356x_usbplug_v1.17.bin",
        "provenance/rkbin-README.md",
    },
    "yy3568": {
        "BOARD.md",
        "BUILD-INFO.txt",
        "LICENSE",
        "README.md",
        "SHA256SUMS",
        "firmware/demo_yy3568.bin",
        "firmware/yy3568.bin",
        "images/demo_yy3568.img",
        "images/yy3568.img",
        "licenses/ROCKCHIP-BINARY-LICENSE",
        "loaders/rk3568_ddr_1560MHz_v1.25.bin",
        "loaders/rk356x_usbplug_v1.17.bin",
        "provenance/rkbin-README.md",
    },
    "rock3a": {
        "BOARD.md",
        "BUILD-INFO.txt",
        "LICENSE",
        "README.md",
        "SHA256SUMS",
        "firmware/demo_rock3a.bin",
        "firmware/rock3a.bin",
        "images/demo_rock3a.img",
        "images/rock3a.img",
        "licenses/ROCKCHIP-BINARY-LICENSE",
        "loaders/rk3568_ddr_1560MHz_v1.25.bin",
        "loaders/rk356x_usbplug_v1.17.bin",
        "provenance/rkbin-README.md",
    },
}


def chainload_files(board: str) -> set[str]:
    return {
        "chainloading.md",
        "chainload/MANIFEST.json",
        "chainload/README.md",
        f"chainload/uboot_{board}.bin",
        f"chainload/uboot_{board}.img",
        f"chainload/uboot_{board}_idbloader.img",
        f"chainload/uboot_{board}_spi.img",
        f"chainload/{board}-u-boot.itb",
        "install/check-partition-overlap.py",
        "install/flash-chainload.sh",
        "loaders/rk3568_bl31_v1.46.elf",
        f"sources/{board}-u-boot-source.tar.xz",
    }


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def run_packager(output: pathlib.Path, version: str = VERSION,
                 extra_env: dict[str, str] | None = None,
                 check: bool = True) -> subprocess.CompletedProcess[str]:
    env = os.environ.copy()
    env["CHAINLOAD_RELEASE"] = "0"
    if extra_env:
        env.update(extra_env)
    return subprocess.run(
        ["bash", str(PACKAGER), version, str(output)],
        cwd=ROOT,
        env=env,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=check,
    )


def parse_sums(data: bytes) -> dict[str, str]:
    result: dict[str, str] = {}
    for raw_line in data.decode("utf-8").splitlines():
        require(len(raw_line) >= 67 and raw_line[64:66] in ("  ", " *"),
                f"invalid SHA256SUMS line: {raw_line}")
        digest = raw_line[:64]
        name = raw_line[66:]
        require(len(digest) == 64, f"invalid SHA-256 digest: {digest}")
        require(name not in result, f"duplicate checksum entry: {name}")
        result[name] = digest
    return result


def check_archive(archive: pathlib.Path, slug: str, commit: str,
                  source_date_epoch: int, chainload: bool = False) -> None:
    root_name = f"rk-{VERSION}-{slug}"
    expected = set(PACKAGES[slug])
    if chainload and slug in ("yy3568", "rock3a"):
        expected.update(chainload_files(slug))
    with tarfile.open(archive, "r:xz") as bundle:
        members = bundle.getmembers()
        files: dict[str, bytes] = {}
        modes: dict[str, int] = {}
        for member in members:
            path = pathlib.PurePosixPath(member.name)
            require(not path.is_absolute(), f"{archive.name}: absolute archive path")
            require(".." not in path.parts, f"{archive.name}: parent path in archive")
            require(path.parts and path.parts[0] == root_name,
                    f"{archive.name}: multiple top-level roots")
            require(member.isdir() or member.isfile(),
                    f"{archive.name}: links and special files are forbidden")
            require(int(member.mtime) == source_date_epoch,
                    f"{archive.name}: non-reproducible timestamp")
            if member.isfile():
                extracted = bundle.extractfile(member)
                require(extracted is not None, f"{member.name}: cannot read archive member")
                relative = path.relative_to(root_name).as_posix()
                files[relative] = extracted.read()
                modes[relative] = member.mode

    require(set(files) == expected,
            f"{archive.name}: file allowlist mismatch: "
            f"missing={sorted(expected - set(files))}, "
            f"extra={sorted(set(files) - expected)}")

    prohibited = ("makeboot.out", "rock.out", "xrock", "rkdeveloptool",
                  "opi5.bin", "opi5.img")
    for name in files:
        allowed_bl31 = name == "loaders/rk3568_bl31_v1.46.elf"
        require(allowed_bl31 or not name.endswith((".elf", ".o", ".out")),
                f"{archive.name}: build product leaked: {name}")
        require(pathlib.PurePosixPath(name).name not in prohibited,
                f"{archive.name}: excluded artifact leaked: {name}")

    internal = parse_sums(files["SHA256SUMS"])
    checksummed = {f"./{name}" for name in files if name != "SHA256SUMS"}
    require(set(internal) == checksummed,
            f"{archive.name}: internal checksum allowlist mismatch")
    for name, digest in internal.items():
        require(sha256(files[name.removeprefix("./")]) == digest,
                f"{archive.name}: internal checksum mismatch for {name}")

    build_info = files["BUILD-INFO.txt"].decode("utf-8")
    require(f"version={VERSION}\n" in build_info,
            f"{archive.name}: version missing from BUILD-INFO")
    require(f"commit={commit}\n" in build_info,
            f"{archive.name}: commit missing from BUILD-INFO")
    if chainload and slug in ("yy3568", "rock3a"):
        require(sha256(files["loaders/rk3568_bl31_v1.46.elf"]) ==
                "c81ac7e8e1fd727cf7f0db62a9aaea760bde2b270e34d98eb264a264b86df749",
                f"{slug} release contains an unpinned BL31")
        with tarfile.open(fileobj=io.BytesIO(
                files[f"sources/{slug}-u-boot-source.tar.xz"]), mode="r:xz") as source:
            names = [pathlib.PurePosixPath(item.name) for item in source.getmembers()]
            require(names and all(not name.is_absolute() and name.parts[0] ==
                                  f"{slug}-u-boot-source" for name in names),
                    "corresponding U-Boot source archive has an unsafe root")
            if slug == "rock3a":
                require(any(name.name == "README.rk-chainload" for name in names),
                        "ROCK 3A corresponding source lacks rebuild instructions")
        require(modes["install/flash-chainload.sh"] == 0o755 and
                modes["install/check-partition-overlap.py"] == 0o755,
                f"{slug} release installer is not executable")


def check_distribution(output: pathlib.Path, chainload: bool = False) -> None:
    archive_names = {
        f"rk-{VERSION}-{slug}.tar.xz" for slug in PACKAGES
    }
    expected_assets = archive_names | {"SHA256SUMS"}
    actual_assets = {path.name for path in output.iterdir()}
    require(actual_assets == expected_assets,
            f"release asset allowlist mismatch: {sorted(actual_assets)}")

    outer = parse_sums((output / "SHA256SUMS").read_bytes())
    require(set(outer) == archive_names, "top-level checksum allowlist mismatch")
    for name, digest in outer.items():
        require(sha256((output / name).read_bytes()) == digest,
                f"top-level checksum mismatch for {name}")

    commit = subprocess.run(
        ["git", "rev-parse", "HEAD"], cwd=ROOT, check=True,
        text=True, stdout=subprocess.PIPE,
    ).stdout.strip().lower()
    epoch = int(subprocess.run(
        ["git", "show", "-s", "--format=%ct", "HEAD"], cwd=ROOT, check=True,
        text=True, stdout=subprocess.PIPE,
    ).stdout.strip())
    for slug in PACKAGES:
        check_archive(output / f"rk-{VERSION}-{slug}.tar.xz", slug, commit, epoch,
                      chainload)


def make_chainload_source(parent: pathlib.Path) -> pathlib.Path:
    destination = parent / "chainload-source"
    shutil.copytree(ROOT, destination, ignore=shutil.ignore_patterns(
        ".git", "dist", "build", "*-u-boot.itb", "uboot_*",
        "*-u-boot-source.tar.xz",
    ))
    for board in ("yy3568", "rock3a"):
        fit = f"synthetic, parser-tested {board} FIT\n".encode()
        stage = (ROOT / "build" / "chainload" / board / "stage.bin").read_bytes()
        (destination / f"{board}-u-boot.itb").write_bytes(fit)
        (destination / f"uboot_{board}.bin").write_bytes(stage + fit)
        (destination / f"uboot_{board}.img").write_bytes(b"synthetic RKNS image\n")
        (destination / f"uboot_{board}_idbloader.img").write_bytes(
            b"synthetic RKNS eMMC ID block\n")
        (destination / f"uboot_{board}_spi.img").write_bytes(
            b"synthetic Rockchip SPI-NOR image\n")
        source_archive = destination / f"{board}-u-boot-source.tar.xz"
        payload = b"synthetic corresponding-source fixture\n"
        with tarfile.open(source_archive, "w:xz") as archive:
            readme = "README.rk-chainload" if board == "rock3a" else "README"
            info = tarfile.TarInfo(f"{board}-u-boot-source/{readme}")
            info.size = len(payload)
            info.mtime = 1
            info.mode = 0o644
            archive.addfile(info, io.BytesIO(payload))
    return destination


def check_reproducibility(first: pathlib.Path, second: pathlib.Path) -> None:
    first_files = {path.name: path.read_bytes() for path in first.iterdir()}
    second_files = {path.name: path.read_bytes() for path in second.iterdir()}
    require(first_files.keys() == second_files.keys(),
            "repeated packaging changed the asset list")
    for name in first_files:
        require(first_files[name] == second_files[name],
                f"repeated packaging changed {name}")


def check_failures(parent: pathlib.Path) -> None:
    invalid = run_packager(parent / "invalid", "1.2.3", check=False)
    require(invalid.returncode != 0 and "vMAJOR.MINOR.PATCH" in invalid.stderr,
            "invalid version was not rejected")
    require(not (parent / "invalid").exists(),
            "invalid version created an output directory")

    occupied = parent / "occupied"
    occupied.mkdir()
    sentinel = occupied / "keep.txt"
    sentinel.write_text("do not overwrite\n", encoding="utf-8")
    nonempty = run_packager(occupied, check=False)
    require(nonempty.returncode != 0 and "not empty" in nonempty.stderr,
            "non-empty destination was not rejected")
    require(sentinel.read_text(encoding="utf-8") == "do not overwrite\n",
            "non-empty destination was modified")

    empty_source = parent / "missing-source"
    empty_source.mkdir()
    missing = run_packager(
        parent / "missing-output",
        extra_env={
            "RK_RELEASE_SOURCE_DIR": str(empty_source),
            "RK_RELEASE_COMMIT": "0" * 40,
            "SOURCE_DATE_EPOCH": "1",
        },
        check=False,
    )
    require(missing.returncode != 0 and "missing required regular file" in missing.stderr,
            "missing package input was not rejected")
    missing_output = parent / "missing-output"
    require(missing_output.is_dir() and not any(missing_output.iterdir()),
            "failed packaging exposed partial release assets")

    bad_flag = run_packager(parent / "bad-flag",
                            extra_env={"CHAINLOAD_RELEASE": "yes"}, check=False)
    require(bad_flag.returncode != 0 and "must be auto, 0, or 1" in bad_flag.stderr,
            "invalid chainloader release mode was not rejected")


def check_workflow_contracts() -> None:
    ci = (ROOT / ".github" / "workflows" / "ci.yml").read_text(encoding="utf-8")
    release = (ROOT / ".github" / "workflows" / "release.yml").read_text(
        encoding="utf-8"
    )
    docs = (ROOT / ".github" / "workflows" / "docs.yml").read_text(
        encoding="utf-8"
    )
    require("actions/checkout@v7" in ci and "contents: read" in ci,
            "CI checkout or permissions are not hardened")
    runner_selector = (
        "runs-on: ${{ fromJSON(vars.RK_RUNNER_LABELS || "
        "'[\"self-hosted\",\"Linux\",\"X64\"]') }}"
    )
    require(ci.count(runner_selector) == 2 and
            "pull_request:" not in ci and "mktemp -d" in ci,
            "CI runner selection or master-only trigger is incorrect")
    require(runner_selector in release and runner_selector in docs,
            "release/docs workflows do not use the configurable runner selector")
    require("actions/checkout@v7" in docs and
            "actions/setup-python" not in docs and
            "python3-venv" in docs and "docs-venv/bin/mkdocs" in docs,
            "docs workflow depends on the GitHub-hosted Python tool cache")
    require("branches: [ master ]" in ci and
            "refs/remotes/origin/master" in release and
            "refs/remotes/origin/main" not in release,
            "CI/release workflows do not follow the repository default branch")
    require("persist-credentials: false" in ci and "make SHELL=/bin/bash check" in ci,
            "CI must run checks without persisted credentials")
    for marker in (
        "tags:", "v*.*.*", "^v(0|[1-9][0-9]*)", "contents: write",
        "fetch-depth: 0",
        "git merge-base --is-ancestor", "make SHELL=/bin/bash check",
        "release-dist", "gh release create", "--draft", "gh release edit",
        "gh release delete", "already exists and will not be modified",
        "load_release_record", ".tag_name", "[.id, .draft]",
        "releases/${release_id}", "compare_remote_assets", "gh release download",
        "matches the rebuilt assets; retry is complete",
        "--latest", "cancel-in-progress: false",
    ):
        require(marker in release, f"release workflow lacks contract marker: {marker}")
    require("matrix:" in ci and "yy3568" in ci and "rock3a" in ci and
            "chainload-check" in ci,
            "CI lacks the board-keyed chainloader job")
    for artifact in ("artifacts.idblock", "artifacts.spi_nor",
                     "reference-idblock.img", "reference-spi.img"):
        require(artifact in ci, f"chainloader CI lacks media check: {artifact}")
    require("CHAINLOAD_RELEASE=1" in release and
            "chainload BOARD=yy3568" in release and
            "chainload BOARD=rock3a" in release and
            "rk-${version}-rock3a.tar.xz" in release,
            "release workflow does not explicitly build/package both chainloaders")


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="rk-release-check-") as temporary:
        parent = pathlib.Path(temporary)
        first = parent / "first"
        second = parent / "second"
        run_packager(first)
        check_distribution(first)
        run_packager(second)
        check_distribution(second)
        check_reproducibility(first, second)
        chain_source = make_chainload_source(parent)
        commit = subprocess.run(
            ["git", "rev-parse", "HEAD"], cwd=ROOT, check=True,
            text=True, stdout=subprocess.PIPE,
        ).stdout.strip()
        epoch = subprocess.run(
            ["git", "show", "-s", "--format=%ct", "HEAD"], cwd=ROOT, check=True,
            text=True, stdout=subprocess.PIPE,
        ).stdout.strip()
        chain_env = {
            "CHAINLOAD_RELEASE": "auto",
            "RK_RELEASE_SOURCE_DIR": str(chain_source),
            "RK_RELEASE_COMMIT": commit,
            "SOURCE_DATE_EPOCH": epoch,
        }
        chain_first = parent / "chain-first"
        chain_second = parent / "chain-second"
        run_packager(chain_first, extra_env=chain_env)
        check_distribution(chain_first, chainload=True)
        run_packager(chain_second, extra_env=chain_env)
        check_distribution(chain_second, chainload=True)
        check_reproducibility(chain_first, chain_second)
        (chain_source / "uboot_rock3a.img").unlink()
        partial_auto = run_packager(parent / "chain-partial-auto",
                                    extra_env=chain_env, check=False)
        require(partial_auto.returncode != 0 and
                "chainloader release inputs are incomplete" in partial_auto.stderr,
                "auto packaging accepted a partial board matrix")
        incomplete_env = dict(chain_env)
        incomplete_env["CHAINLOAD_RELEASE"] = "1"
        incomplete = run_packager(parent / "chain-incomplete",
                                  extra_env=incomplete_env, check=False)
        require(incomplete.returncode != 0 and
                "missing required regular file: uboot_rock3a.img" in incomplete.stderr,
                "incomplete chainloader release inputs were not rejected")
        check_failures(parent)
    check_workflow_contracts()
    print("release distribution checks passed")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.CalledProcessError, tarfile.TarError) as error:
        print(f"release check failed: {error}", file=sys.stderr)
        raise SystemExit(1)

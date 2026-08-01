#!/usr/bin/env bash

set -euo pipefail

export LC_ALL=C
export TZ=UTC

die() {
	printf 'release-dist: %s\n' "$*" >&2
	exit 1
}

usage() {
	printf 'usage: %s vMAJOR.MINOR.PATCH OUTPUT_DIR\n' "$0" >&2
	exit 2
}

[[ $# -eq 2 ]] || usage

version=$1
output_dir=$2

if [[ ! $version =~ ^v(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$ ]]; then
	die "version must be stable SemVer in the form vMAJOR.MINOR.PATCH"
fi

[[ -n $output_dir ]] || die "output directory must not be empty"

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
source_dir=${RK_RELEASE_SOURCE_DIR:-$(cd -- "$script_dir/.." && pwd -P)}
chainload_release=${CHAINLOAD_RELEASE:-auto}
[[ $chainload_release == auto || $chainload_release == 0 || $chainload_release == 1 ]] ||
	die "CHAINLOAD_RELEASE must be auto, 0, or 1"
if [[ $chainload_release == auto ]]; then
	chainload_release=0
	chainload_inputs=(
		yy3568-u-boot.itb uboot_yy3568.bin uboot_yy3568.img
		uboot_yy3568_idbloader.img uboot_yy3568_spi.img yy3568-u-boot-source.tar.xz
		rock3a-u-boot.itb uboot_rock3a.bin uboot_rock3a.img
		uboot_rock3a_idbloader.img uboot_rock3a_spi.img rock3a-u-boot-source.tar.xz
	)
	present=0
	for input in "${chainload_inputs[@]}"; do
		[[ -f $source_dir/$input && ! -L $source_dir/$input ]] && present=$((present + 1))
	done
	if [[ $present -eq ${#chainload_inputs[@]} ]]; then
		chainload_release=1
	elif [[ $present -ne 0 ]]; then
		die "chainloader release inputs are incomplete"
	fi
fi

for command in git install find sort xargs sha256sum tar xz mv; do
	command -v "$command" >/dev/null 2>&1 || die "required command is unavailable: $command"
done

if [[ -e $output_dir && ! -d $output_dir ]]; then
	die "output path exists and is not a directory: $output_dir"
fi
if [[ -d $output_dir ]] && [[ -n $(find "$output_dir" -mindepth 1 -maxdepth 1 -print -quit) ]]; then
	die "output directory is not empty: $output_dir"
fi
mkdir -p -- "$output_dir"
output_dir=$(cd -- "$output_dir" && pwd -P)

source_commit=${RK_RELEASE_COMMIT:-}
if [[ -z $source_commit ]]; then
	source_commit=$(git -C "$source_dir" rev-parse --verify HEAD 2>/dev/null) ||
		die "cannot determine source commit"
fi
if [[ ! $source_commit =~ ^[0-9a-fA-F]{40}$ ]]; then
	die "source commit must be a 40-character Git object ID"
fi

source_date_epoch=${SOURCE_DATE_EPOCH:-}
if [[ -z $source_date_epoch ]]; then
	source_date_epoch=$(git -C "$source_dir" show -s --format=%ct HEAD 2>/dev/null) ||
		die "cannot determine source timestamp"
fi
if [[ ! $source_date_epoch =~ ^[0-9]+$ ]]; then
	die "SOURCE_DATE_EPOCH must be an integer"
fi

stage_dir=$(mktemp -d "${TMPDIR:-/tmp}/rk-release.XXXXXXXX")
trap 'rm -rf -- "$stage_dir"' EXIT
archive_dir="$stage_dir/assets"
mkdir -p -- "$archive_dir"

package_root=
build_info=

copy_file() {
	local source=$1
	local destination=$2
	local role=$3

	[[ -f $source_dir/$source && ! -L $source_dir/$source ]] ||
		die "missing required regular file: $source"
	install -D -m 0644 -- "$source_dir/$source" "$package_root/$destination"
	printf '%-45s %s\n' "$destination" "$role" >> "$build_info"
}

copy_executable() {
	local source=$1
	local destination=$2
	local role=$3

	[[ -f $source_dir/$source && ! -L $source_dir/$source ]] ||
		die "missing required regular file: $source"
	install -D -m 0755 -- "$source_dir/$source" "$package_root/$destination"
	printf '%-45s %s\n' "$destination" "$role" >> "$build_info"
}

start_package() {
	local slug=$1
	local board=$2

	package_name="rk-${version}-${slug}"
	package_root="$stage_dir/$package_name"
	mkdir -p -- "$package_root"
	build_info="$package_root/BUILD-INFO.txt"
	{
		printf 'version=%s\n' "$version"
		printf 'commit=%s\n' "${source_commit,,}"
		printf 'source_date_epoch=%s\n' "$source_date_epoch"
		printf 'board=%s\n' "$board"
		printf '\nFiles:\n'
	} > "$build_info"
	copy_file README.md README.md "project overview"
	copy_file LICENSE LICENSE "project license"
}

finish_package() {
	local archive="$archive_dir/$package_name.tar.xz"

	(
		cd -- "$package_root"
		find . -type f ! -name SHA256SUMS -print0 |
			sort -z |
			xargs -0 sha256sum > SHA256SUMS
	)

	tar \
		--sort=name \
		--format=gnu \
		--mtime="@$source_date_epoch" \
		--owner=0 \
		--group=0 \
		--numeric-owner \
		--mode='u+rwX,go+rX,go-w' \
		-C "$stage_dir" \
		-cf - "$package_name" |
		xz --threads=1 --check=crc64 -9e > "$archive"
}

add_chainloader() {
	local board=$1
	local fit="${board}-u-boot.itb"
	local binary="uboot_${board}.bin"
	local image="uboot_${board}.img"
	local idblock="uboot_${board}_idbloader.img"
	local spi="uboot_${board}_spi.img"
	local source="${board}-u-boot-source.tar.xz"

	copy_file "$fit" "chainload/$fit" "pinned BL31/U-Boot FIT"
	copy_file "$binary" "chainload/$binary" "dedicated first stage plus U-Boot FIT"
	copy_file "$image" "chainload/$image" "RKNS v2 chainloader SD image"
	copy_file "$idblock" "chainload/$idblock" "raw RKNS v2 eMMC ID block for LBA 0x40"
	copy_file "$spi" "chainload/$spi" "Rockchip first-2-KiB-of-4-KiB SPI-NOR image"
	copy_file "config/chainload/${board}.json" chainload/MANIFEST.json "chainloader board manifest"
	copy_file config/chainload/README.md chainload/README.md "chainloader porting policy"
	copy_file docs/rk356x/chainloading.md chainloading.md "RK356x chainloading guide"
	copy_executable tools/flash-chainload.sh install/flash-chainload.sh "guarded eMMC/SPI-NOR installer and restore utility"
	copy_executable tools/check-partition-overlap.py install/check-partition-overlap.py "MBR/GPT overlap guard used by the installer"
	copy_file img/rk3568_bl31_v1.46.elf loaders/rk3568_bl31_v1.46.elf "Rockchip BL31 v1.46"
	copy_file "$source" "sources/$source" "patched corresponding U-Boot source"
}

start_package pinebook-pro "Pine64 Pinebook Pro"
copy_file docs/devices/pinebook.md BOARD.md "board documentation"
copy_file pinebook.bin firmware/pinebook.bin "firmware"
copy_file demo_pinebook.bin firmware/demo_pinebook.bin "firmware with demo payload"
copy_file pinebook.img images/pinebook.img "RKNS SD image"
copy_file demo_pinebook.img images/demo_pinebook.img "RKNS SD image with demo payload"
copy_file pinebook-ddr.bin loaders/pinebook-ddr.bin "source-built DDR loader"
copy_file pinebook-poc-ddr.bin loaders/pinebook-poc-ddr.bin "source-built direct/SD DDR loader"
finish_package

start_package genbook "Cool-Pi Genbook"
copy_file docs/devices/genbook.md BOARD.md "board documentation"
copy_file genbook.bin firmware/genbook.bin "firmware"
copy_file demo_genbook.bin firmware/demo_genbook.bin "firmware with demo payload"
copy_file genbook.img images/genbook.img "RKNS v2 SD image"
copy_file genbook_demo.img images/genbook_demo.img "RKNS v2 SD image with demo payload"
copy_file genbook-ddr.bin loaders/genbook-ddr.bin "source-built DDR loader"
finish_package

start_package roc3566 "Firefly ROC-RK3566-PC"
copy_file docs/rk356x/boards/roc3566.md BOARD.md "ROC-RK3566-PC board documentation"
copy_file roc3566.bin firmware/roc3566.bin "firmware"
copy_file demo_roc3566.bin firmware/demo_roc3566.bin "firmware with demo payload"
copy_file roc3566.img images/roc3566.img "RKNS v2 SD image"
copy_file demo_roc3566.img images/demo_roc3566.img "RKNS v2 SD image with demo payload"
copy_file img/rk3566_ddr_1056MHz_v1.25.bin loaders/rk3566_ddr_1056MHz_v1.25.bin "Rockchip DDR loader"
copy_file img/rk356x_usbplug_v1.17.bin loaders/rk356x_usbplug_v1.17.bin "Rockchip xrock USB-plug loader"
copy_file img/LICENSE licenses/ROCKCHIP-BINARY-LICENSE "Rockchip binary license"
copy_file img/README.md provenance/rkbin-README.md "Rockchip binary provenance"
finish_package

start_package yy3568 "Youyeetoo YY3568"
copy_file docs/rk356x/boards/yy3568.md BOARD.md "YY3568 board documentation"
copy_file yy3568.bin firmware/yy3568.bin "firmware"
copy_file demo_yy3568.bin firmware/demo_yy3568.bin "firmware with demo payload"
copy_file yy3568.img images/yy3568.img "RKNS v2 SD image"
copy_file demo_yy3568.img images/demo_yy3568.img "RKNS v2 SD image with demo payload"
copy_file img/rk3568_ddr_1560MHz_v1.25.bin loaders/rk3568_ddr_1560MHz_v1.25.bin "Rockchip DDR loader"
copy_file img/rk356x_usbplug_v1.17.bin loaders/rk356x_usbplug_v1.17.bin "Rockchip xrock USB-plug loader"
copy_file img/LICENSE licenses/ROCKCHIP-BINARY-LICENSE "Rockchip binary license"
copy_file img/README.md provenance/rkbin-README.md "Rockchip binary provenance"
if [[ $chainload_release == 1 ]]; then
	add_chainloader yy3568
fi
finish_package

start_package rock3a "Radxa ROCK 3A"
copy_file docs/rk356x/boards/rock3a.md BOARD.md "ROCK 3A board documentation"
copy_file rock3a.bin firmware/rock3a.bin "firmware"
copy_file demo_rock3a.bin firmware/demo_rock3a.bin "firmware with demo payload"
copy_file rock3a.img images/rock3a.img "RKNS v2 SD image"
copy_file demo_rock3a.img images/demo_rock3a.img "RKNS v2 SD image with demo payload"
copy_file img/rk3568_ddr_1560MHz_v1.25.bin loaders/rk3568_ddr_1560MHz_v1.25.bin "Rockchip DDR loader"
copy_file img/rk356x_usbplug_v1.17.bin loaders/rk356x_usbplug_v1.17.bin "Rockchip xrock USB-plug loader"
copy_file img/LICENSE licenses/ROCKCHIP-BINARY-LICENSE "Rockchip binary license"
copy_file img/README.md provenance/rkbin-README.md "Rockchip binary provenance"
if [[ $chainload_release == 1 ]]; then
	add_chainloader rock3a
fi
finish_package

(
	cd -- "$archive_dir"
	sha256sum \
		"rk-${version}-genbook.tar.xz" \
		"rk-${version}-pinebook-pro.tar.xz" \
		"rk-${version}-roc3566.tar.xz" \
		"rk-${version}-rock3a.tar.xz" \
		"rk-${version}-yy3568.tar.xz" > SHA256SUMS
)

mv -- \
	"$archive_dir/rk-${version}-genbook.tar.xz" \
	"$archive_dir/rk-${version}-pinebook-pro.tar.xz" \
	"$archive_dir/rk-${version}-roc3566.tar.xz" \
	"$archive_dir/rk-${version}-rock3a.tar.xz" \
	"$archive_dir/rk-${version}-yy3568.tar.xz" \
	"$archive_dir/SHA256SUMS" \
	"$output_dir/"

printf 'release-dist: wrote %s\n' "$output_dir"

#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail

board=${1:-}
if [[ -z "$board" || ! "$board" =~ ^[a-z0-9][a-z0-9_-]*$ ]]; then
	echo "usage: $0 BOARD" >&2
	exit 2
fi

repo=$(cd "$(dirname "$0")/.." && pwd)
manifest="$repo/config/chainload/$board.json"
[[ -f "$manifest" ]] || { echo "chainload-media: board '$board' has no manifest" >&2; exit 2; }
manifest_tool="$repo/tools/chainload-manifest.py"
python3 "$manifest_tool" validate "$board"

json() {
	python3 "$manifest_tool" get "$board" "$1"
}

[[ $(json board) == "$board" ]] || { echo "chainload-media: manifest identity mismatch" >&2; exit 2; }
soc=$(json soc)
ddr="$repo/$(json boot_media.ddr)"
combined="$repo/$(json artifacts.binary)"
idblock="$repo/$(json artifacts.idblock)"
spi="$repo/$(json artifacts.spi_nor)"
sd="$repo/$(json artifacts.image)"
build_root=${CHAINLOAD_BUILD_DIR:-"$repo/build/chainload/$board"}
default_mkimage="$build_root/source/tools/mkimage"
mkimage=${MKIMAGE:-$default_mkimage}

[[ -f "$ddr" ]] || { echo "chainload-media: missing DDR image: $ddr" >&2; exit 2; }
[[ -f "$combined" ]] || { echo "chainload-media: missing chainloader image: $combined" >&2; exit 2; }
[[ -x "$mkimage" ]] || {
	echo "chainload-media: pinned mkimage is unavailable: $mkimage" >&2
	echo "build pinned U-Boot first or set MKIMAGE=/path/to/tools/mkimage" >&2
	exit 2
}

tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/rk-chainload-media.XXXXXX")
cleanup() { rm -rf -- "$tmp_dir"; }
trap cleanup EXIT

"$mkimage" -n "$soc" -T rksd -d "$ddr:$combined" "$tmp_dir/idblock.img"
"$mkimage" -n "$soc" -T rkspi -d "$ddr:$combined" "$tmp_dir/spi.img"

# The raw rksd ID block is written at LBA 64 on eMMC. The SD convenience
# image carries the same bytes at that offset so it can be written as a disk.
truncate -s $((64 * 512)) "$tmp_dir/sd.img"
cat "$tmp_dir/idblock.img" >> "$tmp_dir/sd.img"

mv "$tmp_dir/idblock.img" "$idblock"
mv "$tmp_dir/spi.img" "$spi"
mv "$tmp_dir/sd.img" "$sd"
echo "chainload-media: built $(basename "$idblock"), $(basename "$spi"), and $(basename "$sd")"

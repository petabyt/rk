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
if [[ ! -f "$manifest" ]]; then
	echo "chainload: board '$board' has no manifest" >&2
	exit 2
fi

manifest_tool="$repo/tools/chainload-manifest.py"
python3 "$manifest_tool" validate "$board"

json() {
	python3 "$manifest_tool" get "$board" "$1"
}

manifest_board=$(json board)
[[ "$manifest_board" == "$board" ]] || {
	echo "chainload: manifest identity mismatch" >&2
	exit 2
}

fit_name=$(json artifacts.fit)
source_name=$(json artifacts.source)
output="$repo/$fit_name"
source_output="$repo/$source_name"

if [[ -n "${UBOOT_ITB:-}" ]]; then
	prebuilt=$(cd "$(dirname "$UBOOT_ITB")" && pwd)/$(basename "$UBOOT_ITB")
	[[ -f "$prebuilt" ]] || { echo "UBOOT_ITB does not exist: $prebuilt" >&2; exit 2; }
	rm -f -- "$source_output"
	if [[ "$prebuilt" != "$output" ]]; then
		cp "$prebuilt" "$output"
	fi
	echo "chainload: copied development FIT to $fit_name"
	exit 0
fi

rm -f -- "$output" "$source_output"

uboot_repo=$(json uboot.repository)
uboot_backend=$(json uboot.backend)
uboot_ref=$(json uboot.ref)
uboot_commit=$(json uboot.commit)
defconfig=$(json uboot.defconfig)
overlay_rel=$(json uboot.overlay)
bl31_rel=$(json bl31.path)
bl31_hash=$(json bl31.sha256)
bl31_size=$(json bl31.size)
overlay="$repo/$overlay_rel"
bl31="$repo/$bl31_rel"

[[ -d "$overlay" ]] || { echo "chainload: missing board overlay: $overlay_rel" >&2; exit 2; }
[[ -f "$bl31" ]] || { echo "chainload: missing BL31: $bl31_rel" >&2; exit 2; }
actual_hash=$(sha256sum "$bl31" | awk '{print $1}')
actual_size=$(wc -c < "$bl31" | tr -d '[:space:]')
[[ "$actual_hash" == "$bl31_hash" && "$actual_size" == "$bl31_size" ]] || {
	echo "chainload: BL31 provenance check failed" >&2
	exit 2
}

build_root=${CHAINLOAD_BUILD_DIR:-"$repo/build/chainload/$board"}
mkdir -p "$build_root/cache"
build_root=$(cd "$build_root" && pwd)
case "$build_root" in
	/|"$repo"|"$repo/build"|"$repo/build/chainload")
		echo "chainload: refusing unsafe build directory: $build_root" >&2
		exit 2
		;;
esac

if [[ -n "${UBOOT_SRC:-}" ]]; then
	source_git=$(cd "$UBOOT_SRC" && pwd)
	git -C "$source_git" cat-file -e "$uboot_commit^{commit}" 2>/dev/null || {
		echo "chainload: UBOOT_SRC does not contain pinned commit $uboot_commit" >&2
		exit 2
	}
else
	source_git="$build_root/cache/u-boot.git"
	if [[ ! -d "$source_git" ]]; then
		git init --bare "$source_git"
		git -C "$source_git" remote add origin "$uboot_repo"
	fi
	if ! git -C "$source_git" cat-file -e "$uboot_commit^{commit}" 2>/dev/null; then
		git -C "$source_git" fetch --depth=1 origin "$uboot_ref"
		git -C "$source_git" cat-file -e "$uboot_commit^{commit}" 2>/dev/null || {
			echo "chainload: $uboot_ref no longer contains pinned commit $uboot_commit" >&2
			exit 2
		}
	fi
fi

snapshot="$build_root/source"
if [[ -e "$snapshot" ]]; then
	case "$snapshot" in "$build_root"/*) rm -rf -- "$snapshot" ;; *) exit 2 ;; esac
fi
mkdir -p "$snapshot"
git -C "$source_git" archive "$uboot_commit" | tar -x -C "$snapshot"
cp -a "$overlay/." "$snapshot/"
if [[ -f "$snapshot/arch/arm/mach-rockchip/decode_bl31.py" ]]; then
	chmod 0755 "$snapshot/arch/arm/mach-rockchip/decode_bl31.py"
fi

epoch=$(git -C "$source_git" show -s --format=%ct "$uboot_commit")
source_tmp="$build_root/$source_name.tmp"
tar --sort=name --owner=0 --group=0 --numeric-owner \
	--mode='u=rwX,go=rX' --mtime="@$epoch" \
	--transform="s,^\./,$board-u-boot-source/," \
	-c -C "$snapshot" . | xz -9 -T1 > "$source_tmp"
mv "$source_tmp" "$source_output"

jobs=${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)}
cross=${CROSS_COMPILE:-aarch64-linux-gnu-}
export BL31="$bl31"
export ROCKCHIP_TPL="$repo/$(json boot_media.ddr)"
export SOURCE_DATE_EPOCH="$epoch"
export KBUILD_BUILD_TIMESTAMP="@$epoch"
export KBUILD_BUILD_USER=rk-chainload
export KBUILD_BUILD_HOST=github-actions
export GIT_CEILING_DIRECTORIES="$build_root"
make -C "$snapshot" CROSS_COMPILE="$cross" "$defconfig"
case "$uboot_backend" in
vendor-fit)
	# Armbian's pinned RK35xx flow explicitly builds u-boot.dtb before
	# u-boot.itb and demotes diagnostics that modern GCC promotes in this
	# vendor tree. Keep those accommodations inside the vendor backend.
	vendor_cflags=(
		-fdiagnostics-color=always
		-Wno-error=maybe-uninitialized
		-Wno-error=misleading-indentation
		-Wno-error=attributes
		-Wno-error=address-of-packed-member
		-Wno-error=implicit-function-declaration
		-Wno-error=implicit-int
		-Wno-error=int-conversion
		-Wno-error=incompatible-pointer-types
		-Wno-error=array-parameter
	)
	vendor_flags="${vendor_cflags[*]}"
	make -C "$snapshot" CROSS_COMPILE="$cross" \
		CFLAGS="$vendor_flags" KCFLAGS="$vendor_flags" -j"$jobs" \
		u-boot.bin tools u-boot.its u-boot.dtb
	[[ -s "$snapshot/u-boot.dtb" ]] || { echo "chainload: vendor U-Boot created an empty u-boot.dtb" >&2; exit 2; }
	# Use inline FIT data: the dedicated chainloader intentionally rejects
	# the vendor make target's external-data (-E) representation.
	(cd "$snapshot" && ./tools/mkimage -f u-boot.its u-boot.itb)
	fit_listing=$(cd "$snapshot" && ./tools/dumpimage -l u-boot.itb)
	if grep -Eq 'Data Size:[[:space:]]+0 Bytes' <<< "$fit_listing"; then
		echo "chainload: vendor U-Boot FIT contains a zero-byte image" >&2
		exit 2
	fi
	;;
mainline-fit)
	fragment="$repo/$(json uboot.config_fragment)"
	[[ -f "$fragment" ]] || { echo "chainload: missing config fragment" >&2; exit 2; }
	(cd "$snapshot" && bash scripts/kconfig/merge_config.sh -m .config "$fragment")
	make -C "$snapshot" CROSS_COMPILE="$cross" olddefconfig
	make -C "$snapshot" CROSS_COMPILE="$cross" -j"$jobs" \
		all
	;;
*)
	echo "chainload: unsupported U-Boot backend: $uboot_backend" >&2
	exit 2
	;;
esac
[[ -x "$snapshot/tools/mkimage" ]] || { echo "chainload: U-Boot did not build tools/mkimage" >&2; exit 2; }
[[ -f "$snapshot/u-boot.itb" ]] || { echo "chainload: U-Boot did not create u-boot.itb" >&2; exit 2; }
cp "$snapshot/u-boot.itb" "$output"
echo "chainload: built $fit_name and $source_name"

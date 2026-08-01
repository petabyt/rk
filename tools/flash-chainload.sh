#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail

repo=${CHAINLOAD_REPO:-$(cd "$(dirname "$0")/.." && pwd)}
repo=$(cd "$repo" && pwd)
xrock=${XROCK:-xrock}
rkdeveloptool=${RKDEVELOPTOOL:-rkdeveloptool}

die() {
	echo "chainload-flash: $*" >&2
	exit 2
}

need_tool() {
	command -v "$1" >/dev/null 2>&1 || die "required tool not found: $1"
}

json() {
	python3 - "$1" "$2" <<'PY'
import json, sys
value = json.load(open(sys.argv[1], encoding="utf-8"))
for component in sys.argv[2].split("."):
    value = value[component]
print(value)
PY
}

find_manifest() {
	local board=$1 primary bundled
	primary="$repo/config/chainload/$board.json"
	bundled="$repo/chainload/MANIFEST.json"
	if [[ -f "$primary" && ! -L "$primary" ]]; then
		printf '%s\n' "$primary"
	elif [[ -f "$bundled" && ! -L "$bundled" ]] && [[ $(json "$bundled" board) == "$board" ]]; then
		printf '%s\n' "$bundled"
	else
		return 1
	fi
}

resolve_input() {
	local relative=$1 candidate
	for candidate in "$repo/$relative" "$repo/chainload/$(basename "$relative")" \
			"$repo/loaders/$(basename "$relative")"; do
		if [[ -f "$candidate" && ! -L "$candidate" ]]; then
			printf '%s\n' "$candidate"
			return
		fi
	done
	return 1
}

partition_checker() {
	local candidate
	for candidate in "$repo/tools/check-partition-overlap.py" \
			"$repo/install/check-partition-overlap.py"; do
		[[ -f "$candidate" && ! -L "$candidate" ]] && { printf '%s\n' "$candidate"; return; }
	done
	return 1
}

require_sector_file() {
	local file=$1 sectors=$2 description=$3 actual
	[[ -f "$file" && ! -L "$file" ]] || die "$description was not created as a regular file"
	actual=$(wc -c < "$file" | tr -d '[:space:]')
	(( actual == sectors * 512 )) || die "$description has an unexpected size"
}

record_jedec_id() {
	local destination=$1 output id manufacturer
	output=$("$rkdeveloptool" rid)
	printf '%s\n' "$output" | tee "$destination"
	id=$(printf '%s\n' "$output" |
		sed -n 's/^Flash ID: \([0-9A-F][0-9A-F]\( [0-9A-F][0-9A-F]\)\{4\}\).*/\1/p' |
		tail -n 1)
	[[ -n "$id" ]] || die "SPI NOR JEDEC identification failed"
	manufacturer=${id%% *}
	[[ "$manufacturer" != 00 && "$manufacturer" != FF ]] ||
		die "SPI NOR returned an invalid JEDEC manufacturer ID"
}

sha256_of() {
	sha256sum "$1" | awk '{print $1}'
}

device_list() {
	"$rkdeveloptool" ld
}

require_one_rk356x() {
	local listing count
	listing=$(device_list)
	count=$(printf '%s\n' "$listing" | grep -Eic 'Vid=0x2207')
	[[ $count -eq 1 ]] || die "expected exactly one Rockchip device, found $count"
	printf '%s\n' "$listing" | grep -Eqi 'Pid=0x350a' || die "connected Rockchip device is not RK356x PID 0x350a"
	printf '%s\n' "$listing"
}

enter_loader() {
	local ddr=$1 usbplug=$2 listing ready=0
	listing=$(require_one_rk356x)
	if printf '%s\n' "$listing" | grep -Eqi 'Mode=Loader'; then
		echo "chainload-flash: device is already in loader mode"
		return
	fi
	printf '%s\n' "$listing" | grep -Eqi 'Mode=Maskrom' || die "device is neither MaskROM nor loader mode"
	"$xrock" maskrom "$ddr" "$usbplug" --rc4-off
	for _ in $(seq 1 20); do
		listing=$(device_list 2>/dev/null || true)
		if [[ $(printf '%s\n' "$listing" | grep -Eic 'Vid=0x2207') -eq 1 ]] &&
			printf '%s\n' "$listing" | grep -Eqi 'Pid=0x350a.*Mode=Loader'; then
			ready=1
			break
		fi
		sleep 0.25
	done
	[[ $ready -eq 1 ]] || die "RK356x did not enter loader mode"
}

select_storage() {
	local storage_id=$1 output
	output=$("$rkdeveloptool" cs "$storage_id")
	printf '%s\n' "$output"
	printf '%s\n' "$output" | grep -q 'Change Storage OK' || die "storage selection was not confirmed"
}

flash_info() {
	local output
	output=$("$rkdeveloptool" rfi)
	printf '%s\n' "$output" | tee "$1" >&2
	printf '%s\n' "$output" | sed -n 's/.*Flash Size: \([0-9][0-9]*\) Sectors.*/\1/p' | tail -n 1
}

write_backup_manifest() {
	python3 - "$@" <<'PY'
import hashlib, json, pathlib, sys
out, board, media, storage, start, installed_sectors, capacity, backup_file, backup_sectors, installed, manifest = sys.argv[1:]
backup = pathlib.Path(out).parent / backup_file
payload = pathlib.Path(installed)
doc = {
    "schema": 2,
    "board": board,
    "media": media,
    "storage_id": int(storage),
    "write_lba": int(start),
    "installed_sectors": int(installed_sectors),
    "capacity_sectors": int(capacity),
    "backup_file": backup_file,
    "backup_sectors": int(backup_sectors),
    "backup_sha256": hashlib.sha256(backup.read_bytes()).hexdigest(),
    "installed_sha256": hashlib.sha256(payload.read_bytes()).hexdigest(),
    "manifest_sha256": hashlib.sha256(pathlib.Path(manifest).read_bytes()).hexdigest(),
}
pathlib.Path(out).write_text(json.dumps(doc, indent=2, sort_keys=True) + "\n", encoding="utf-8")
PY
}

prepare_backup_dir() {
	local destination=$1
	case "$destination" in /*|[A-Za-z]:/*) ;; *) die "BACKUP_DIR must be an absolute path" ;; esac
	[[ ! -L "$destination" ]] || die "BACKUP_DIR may not be a symbolic link"
	if [[ -e "$destination" ]]; then
		[[ -d "$destination" ]] || die "BACKUP_DIR is not a directory"
		[[ -z $(find "$destination" -mindepth 1 -maxdepth 1 -print -quit) ]] || die "BACKUP_DIR must be empty"
	else
		mkdir -p -- "$destination"
	fi
}

flash_media() {
	local board=$1 media=$2 backup_dir=$3 confirm=$4
	[[ "$confirm" == "$board:$media" ]] || die "CONFIRM must equal '$board:$media'"
	[[ "$board" =~ ^[a-z0-9][a-z0-9_-]*$ ]] || die "invalid board identifier"
	[[ "$media" == emmc || "$media" == spi-nor ]] || die "MEDIA must be emmc or spi-nor"
	need_tool python3
	local manifest artifact ddr usbplug storage start bytes sectors capacity info_file backup_file backup_sectors checker
	manifest=$(find_manifest "$board") || die "board '$board' has no chainload manifest"
	[[ $(json "$manifest" board) == "$board" ]] || die "chainload manifest identity mismatch"
	artifact=$(resolve_input "$(json "$manifest" "boot_media.$media.artifact")") || die "media image is missing"
	ddr=$(resolve_input "$(json "$manifest" boot_media.ddr)") || die "DDR loader is missing"
	usbplug=$(resolve_input "$(json "$manifest" boot_media.usbplug)") || die "USB-plug loader is missing"
	storage=$(json "$manifest" "boot_media.$media.storage_id")
	start=$(json "$manifest" "boot_media.$media.write_lba")
	bytes=$(wc -c < "$artifact" | tr -d '[:space:]')
	(( bytes > 0 && bytes % 512 == 0 )) || die "media image is not sector aligned"
	sectors=$((bytes / 512))

	need_tool "$xrock"
	need_tool "$rkdeveloptool"
	need_tool sha256sum
	"$rkdeveloptool" -h 2>&1 | grep -q 'ChangeStorage' || die "rkdeveloptool lacks storage selection support"
	prepare_backup_dir "$backup_dir"
	trap 'echo "chainload-flash: operation failed; the device was not reset" >&2; echo "chainload-flash: backup directory: '"$backup_dir"'" >&2' ERR
	enter_loader "$ddr" "$usbplug"
	require_one_rk356x >/dev/null
	select_storage "$storage"
	info_file="$backup_dir/flash-info.txt"
	capacity=$(flash_info "$info_file")
	[[ "$capacity" =~ ^[0-9]+$ && $capacity -gt 0 ]] || die "unable to determine selected-storage capacity"
	(( start >= 0 && start + sectors <= capacity )) ||
		die "media image range exceeds selected-storage capacity"

	if [[ "$media" == emmc ]]; then
		backup_file=previous-idblock-region.bin
		backup_sectors=$sectors
		"$rkdeveloptool" rl 0 64 "$backup_dir/emmc-lba0-63.bin"
		"$rkdeveloptool" rl "$start" "$sectors" "$backup_dir/$backup_file"
		require_sector_file "$backup_dir/emmc-lba0-63.bin" 64 "eMMC metadata backup"
		require_sector_file "$backup_dir/$backup_file" "$sectors" "eMMC destination backup"
		checker=$(partition_checker) || die "partition-overlap checker is missing"
		python3 "$checker" \
			"$backup_dir/emmc-lba0-63.bin" "$start" "$sectors"
	else
		backup_file=complete-spi-nor.bin
		backup_sectors=$capacity
		record_jedec_id "$backup_dir/flash-id.txt"
		"$rkdeveloptool" rl 0 "$capacity" "$backup_dir/$backup_file"
		require_sector_file "$backup_dir/$backup_file" "$capacity" "complete SPI-NOR backup"
	fi

	write_backup_manifest "$backup_dir/backup.json" "$board" "$media" "$storage" \
		"$start" "$sectors" "$capacity" "$backup_file" "$backup_sectors" "$artifact" "$manifest"
	sha256sum "$backup_dir/$backup_file" "$artifact" > "$backup_dir/SHA256SUMS"
	"$rkdeveloptool" wl "$start" "$artifact"
	"$rkdeveloptool" rl "$start" "$sectors" "$backup_dir/installed-readback.bin"
	require_sector_file "$backup_dir/installed-readback.bin" "$sectors" "installed-image readback"
	if [[ $(sha256_of "$artifact") != "$(sha256_of "$backup_dir/installed-readback.bin")" ]] ||
		! cmp "$artifact" "$backup_dir/installed-readback.bin"; then
		echo "chainload-flash: write verification mismatch; the device remains in loader mode" >&2
		echo "chainload-flash: restore with: make restore-chainload BACKUP=$backup_dir CONFIRM=restore:$board:$media" >&2
		exit 2
	fi
	sha256sum "$backup_dir/installed-readback.bin" >> "$backup_dir/SHA256SUMS"
	trap - ERR
	"$rkdeveloptool" rd
	echo "chainload-flash: $board $media installed and verified"
}

restore_media() {
	local backup_dir=$1 confirm=$2 manifest
	manifest="$backup_dir/backup.json"
	case "$backup_dir" in /*|[A-Za-z]:/*) ;; *) die "BACKUP must be an absolute path" ;; esac
	[[ ! -L "$backup_dir" ]] || die "BACKUP may not be a symbolic link"
	[[ -f "$manifest" && ! -L "$manifest" ]] || die "backup.json is missing or unsafe"
	need_tool python3
	need_tool sha256sum
	local schema board media storage start installed_sectors backup_file backup_sectors expected_hash expected_capacity expected_manifest_hash ddr usbplug source bytes capacity info_file
	schema=$(json "$manifest" schema)
	[[ "$schema" == 2 ]] || die "unsupported backup metadata schema"
	board=$(json "$manifest" board)
	media=$(json "$manifest" media)
	[[ "$board" =~ ^[a-z0-9][a-z0-9_-]*$ ]] || die "backup contains an invalid board identifier"
	[[ "$media" == emmc || "$media" == spi-nor ]] || die "backup contains an invalid media identifier"
	[[ "$confirm" == "restore:$board:$media" ]] || die "CONFIRM must equal 'restore:$board:$media'"
	storage=$(json "$manifest" storage_id)
	start=$(json "$manifest" write_lba)
	installed_sectors=$(json "$manifest" installed_sectors)
	backup_file=$(json "$manifest" backup_file)
	backup_sectors=$(json "$manifest" backup_sectors)
	expected_hash=$(json "$manifest" backup_sha256)
	expected_capacity=$(json "$manifest" capacity_sectors)
	expected_manifest_hash=$(json "$manifest" manifest_sha256)
	[[ "$storage" =~ ^[0-9]+$ && "$start" =~ ^[0-9]+$ &&
		"$installed_sectors" =~ ^[0-9]+$ &&
		"$backup_sectors" =~ ^[0-9]+$ && "$expected_capacity" =~ ^[0-9]+$ ]] ||
		die "backup contains invalid numeric fields"
	(( installed_sectors > 0 && backup_sectors > 0 && expected_capacity > 0 &&
		start + installed_sectors <= expected_capacity )) ||
		die "backup contains an invalid saved range"
	[[ "$expected_hash" =~ ^[0-9a-f]{64}$ ]] || die "backup contains an invalid checksum"
	[[ "$expected_manifest_hash" =~ ^[0-9a-f]{64}$ ]] || die "backup contains an invalid manifest checksum"
	if [[ "$media" == emmc ]]; then
		[[ "$backup_file" == previous-idblock-region.bin &&
			"$backup_sectors" == "$installed_sectors" ]] ||
			die "eMMC backup saved range differs from the selected target"
	else
		[[ "$backup_file" == complete-spi-nor.bin && "$start" == 0 &&
			"$backup_sectors" == "$expected_capacity" ]] ||
			die "SPI-NOR backup saved range differs from the selected target"
	fi
	source="$backup_dir/$backup_file"
	[[ -f "$source" && ! -L "$source" ]] || die "backup payload is missing or unsafe"
	bytes=$(wc -c < "$source" | tr -d '[:space:]')
	(( bytes == backup_sectors * 512 )) || die "backup payload size does not match its manifest"
	[[ $(sha256_of "$source") == "$expected_hash" ]] || die "backup payload checksum mismatch"
	local board_manifest
	board_manifest=$(find_manifest "$board") || die "board '$board' has no chainload manifest"
	[[ $(sha256_of "$board_manifest") == "$expected_manifest_hash" ]] ||
		die "backup was created for a different board manifest"
	[[ "$storage" == "$(json "$board_manifest" "boot_media.$media.storage_id")" ]] ||
		die "backup storage ID conflicts with the board manifest"
	[[ "$start" == "$(json "$board_manifest" "boot_media.$media.write_lba")" ]] ||
		die "backup write offset conflicts with the board manifest"
	ddr=$(resolve_input "$(json "$board_manifest" boot_media.ddr)") || die "DDR loader is missing"
	usbplug=$(resolve_input "$(json "$board_manifest" boot_media.usbplug)") || die "USB-plug loader is missing"
	need_tool "$xrock"
	need_tool "$rkdeveloptool"
	"$rkdeveloptool" -h 2>&1 | grep -q 'ChangeStorage' || die "rkdeveloptool lacks storage selection support"
	enter_loader "$ddr" "$usbplug"
	require_one_rk356x >/dev/null
	select_storage "$storage"
	info_file="$backup_dir/restore-flash-info.txt"
	capacity=$(flash_info "$info_file")
	[[ "$capacity" =~ ^[0-9]+$ && $capacity -gt 0 ]] || die "unable to determine selected-storage capacity"
	[[ "$capacity" == "$expected_capacity" ]] || die "selected-storage capacity differs from the backup"
	(( start >= 0 && start + backup_sectors <= capacity )) ||
		die "backup range exceeds selected-storage capacity"
	if [[ "$media" == spi-nor ]]; then
		record_jedec_id "$backup_dir/restore-flash-id.txt"
	fi
	trap 'echo "chainload-flash: restore failed; the device was not reset" >&2' ERR
	"$rkdeveloptool" wl "$start" "$source"
	"$rkdeveloptool" rl "$start" "$backup_sectors" "$backup_dir/restore-readback.bin"
	require_sector_file "$backup_dir/restore-readback.bin" "$backup_sectors" "restore readback"
	if [[ $(sha256_of "$source") != "$(sha256_of "$backup_dir/restore-readback.bin")" ]] ||
		! cmp "$source" "$backup_dir/restore-readback.bin"; then
		die "restore verification mismatch; the device remains in loader mode"
	fi
	trap - ERR
	"$rkdeveloptool" rd
	echo "chainload-flash: restored $board $media from $backup_file"
}

case ${1:-} in
flash)
	[[ $# -eq 5 ]] || die "usage: $0 flash BOARD MEDIA BACKUP_DIR CONFIRM"
	flash_media "$2" "$3" "$4" "$5"
	;;
restore)
	[[ $# -eq 3 ]] || die "usage: $0 restore BACKUP_DIR CONFIRM"
	restore_media "$2" "$3"
	;;
*)
	die "usage: $0 flash BOARD MEDIA BACKUP_DIR CONFIRM | restore BACKUP_DIR CONFIRM"
	;;
esac

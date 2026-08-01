#!/usr/bin/env python3
"""Reject an eMMC ID-block write that overlaps an existing MBR/GPT partition."""

from __future__ import annotations

import pathlib
import struct
import sys
import zlib


def fail(message: str) -> "NoReturn":
    raise ValueError(message)


def overlaps(first: int, last: int, target_first: int, target_last: int) -> bool:
    return first <= target_last and target_first <= last


def check_gpt(data: bytes, target_first: int, target_last: int) -> bool:
    sector = 512
    if len(data) < sector * 2 or data[sector:sector + 8] != b"EFI PART":
        return False
    header = data[sector:sector * 2]
    header_size, header_crc = struct.unpack_from("<II", header, 12)
    if not 92 <= header_size <= sector:
        fail("invalid GPT header size")
    checked = bytearray(header[:header_size])
    checked[16:20] = b"\0" * 4
    if zlib.crc32(checked) & 0xFFFFFFFF != header_crc:
        fail("invalid GPT header CRC")
    entry_lba = struct.unpack_from("<Q", header, 72)[0]
    entry_count, entry_size, entries_crc = struct.unpack_from("<III", header, 80)
    if entry_size < 128 or entry_size > 4096 or entry_count > 4096:
        fail("unsupported GPT entry table")
    table_start = entry_lba * sector
    table_size = entry_count * entry_size
    table_end = table_start + table_size
    if table_end > len(data):
        fail("GPT entry table is outside the backed-up metadata")
    table = data[table_start:table_end]
    if zlib.crc32(table) & 0xFFFFFFFF != entries_crc:
        fail("invalid GPT entry-table CRC")
    for index in range(entry_count):
        entry = table[index * entry_size:(index + 1) * entry_size]
        if entry[:16] == b"\0" * 16:
            continue
        first, last = struct.unpack_from("<QQ", entry, 32)
        if first > last:
            fail(f"GPT partition {index + 1} has an invalid range")
        if overlaps(first, last, target_first, target_last):
            fail(
                f"GPT partition {index + 1} ({first}-{last}) overlaps "
                f"the firmware range {target_first}-{target_last}"
            )
    print("eMMC GPT partitions do not overlap the firmware range")
    return True


def check_mbr(data: bytes, target_first: int, target_last: int) -> bool:
    if len(data) < 512 or data[510:512] != b"\x55\xaa":
        return False
    found = False
    for index in range(4):
        entry = data[446 + index * 16:446 + (index + 1) * 16]
        kind = entry[4]
        first, count = struct.unpack_from("<II", entry, 8)
        if kind == 0 or count == 0 or kind == 0xEE:
            continue
        found = True
        last = first + count - 1
        if overlaps(first, last, target_first, target_last):
            fail(
                f"MBR partition {index + 1} ({first}-{last}) overlaps "
                f"the firmware range {target_first}-{target_last}"
            )
    if found:
        print("eMMC MBR partitions do not overlap the firmware range")
    return True


def main() -> int:
    if len(sys.argv) != 4:
        print(f"usage: {sys.argv[0]} METADATA START_LBA SECTORS", file=sys.stderr)
        return 2
    data = pathlib.Path(sys.argv[1]).read_bytes()
    start = int(sys.argv[2], 0)
    sectors = int(sys.argv[3], 0)
    if start < 0 or sectors <= 0:
        fail("invalid firmware LBA range")
    last = start + sectors - 1
    gpt = check_gpt(data, start, last)
    mbr = check_mbr(data, start, last)
    if not gpt and not mbr:
        print("no MBR/GPT partition table detected in the eMMC metadata")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"partition check failed: {error}", file=sys.stderr)
        raise SystemExit(1)

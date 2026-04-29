#!/usr/bin/env python3
"""OTA and bootloader test tool for L8_OTA.

This script validates the OTA packet format for the application code and
checks the bootloader address/jump logic in Bootloader/Src/main.c.
"""

import argparse
import math
import re
from pathlib import Path

OTA_HEADER = 0xA5
OTA_ACK = 0x06
OTA_NACK = 0x15
OTA_START = 0x01
OTA_DATA = 0x02
OTA_END = 0x03
MAX_PAYLOAD = 128
PACKET_SIZE = 2 + 2 + 1 + 1 + 128 + 4

APP1_ADDR = 0x08004000
APP2_ADDR = 0x08012000


def stm32_crc32(data: bytes) -> int:
    poly = 0x04C11DB7
    crc = 0xFFFFFFFF
    padded_len = (len(data) + 3) & ~3
    padded = data + b"\x00" * (padded_len - len(data))
    for i in range(0, len(padded), 4):
        word = int.from_bytes(padded[i:i+4], "little")
        for bit in range(32):
            data_bit = (word >> (31 - bit)) & 1
            crc_msb = (crc >> 31) & 1
            crc = ((crc << 1) & 0xFFFFFFFF)
            if crc_msb ^ data_bit:
                crc ^= poly
    return crc & 0xFFFFFFFF


def build_ota_packet(seq: int, command: int, payload: bytes) -> bytes:
    if len(payload) > MAX_PAYLOAD:
        raise ValueError("payload too large")
    payload_padded = payload.ljust(MAX_PAYLOAD, b"\x00")
    header = OTA_HEADER.to_bytes(2, "little")
    seq_bytes = seq.to_bytes(2, "little")
    command_byte = command.to_bytes(1, "little")
    length_byte = len(payload).to_bytes(1, "little")
    packet = header + seq_bytes + command_byte + length_byte + payload_padded
    crc = stm32_crc32(packet)
    return packet + crc.to_bytes(4, "little")


def make_start_packet(firmware: bytes, version: int = 1) -> bytes:
    total_bytes = len(firmware)
    total_frames = math.ceil(total_bytes / MAX_PAYLOAD)
    crc32 = stm32_crc32(firmware)
    metadata = (
        version.to_bytes(4, "little")
        + total_bytes.to_bytes(4, "little")
        + total_frames.to_bytes(2, "little")
        + crc32.to_bytes(4, "little")
    )
    metadata = metadata.ljust(MAX_PAYLOAD, b"\x00")
    return build_ota_packet(seq=0, command=OTA_START, payload=metadata)


def make_data_packets(firmware: bytes) -> list[bytes]:
    packets = []
    for index in range(0, len(firmware), MAX_PAYLOAD):
        chunk = firmware[index : index + MAX_PAYLOAD]
        seq = len(packets) + 1
        packets.append(build_ota_packet(seq=seq, command=OTA_DATA, payload=chunk))
    return packets


def make_end_packet(seq: int) -> bytes:
    return build_ota_packet(seq=seq, command=OTA_END, payload=b"")


def verify_bootloader_source(path: Path) -> dict:
    text = path.read_text()
    result = {
        "app1_addr": bool(re.search(r"ADDRESS_FLAG_APP1\s+0x08004000U", text)),
        "app2_addr": bool(re.search(r"ADDRESS_FLAG_APP2\s+0x08012000U", text)),
        "jump_msp": bool(re.search(r"__set_MSP\(\*\(volatile uint32_t \*\)\s*app_address\)", text)),
        "jump_handler": bool(re.search(r"app_address\s*\+\s*4", text)),
        "button_logic": bool(re.search(r"GPIOC_IDR\s*&\s*\(1U\s*<<\s*13\).*ADDRESS_FLAG_APP2", text, re.S))
    }
    return result


def report(firmware_path: Path, bootloader_path: Path, dump_packet: bool = False) -> int:
    firmware = firmware_path.read_bytes()
    total_bytes = len(firmware)
    total_frames = math.ceil(total_bytes / MAX_PAYLOAD)
    start_packet = make_start_packet(firmware)
    data_packets = make_data_packets(firmware)
    end_packet = make_end_packet(seq=len(data_packets) + 1)

    print(f"Firmware: {firmware_path}")
    print(f"  Size: {total_bytes} bytes")
    print(f"  Frames (128B max): {total_frames}")
    print(f"  Start packet size: {len(start_packet)}")
    print(f"  Data packets: {len(data_packets)}")
    print(f"  End packet size: {len(end_packet)}")
    print(f"  Total OTA bytes: {len(start_packet) + sum(len(p) for p in data_packets) + len(end_packet)}")

    # Validate packet structure
    assert len(start_packet) == PACKET_SIZE
    assert all(len(p) == PACKET_SIZE for p in data_packets)
    assert len(end_packet) == PACKET_SIZE

    print("\nStart packet metadata:")
    print(f"  Version: 1")
    print(f"  Total bytes: {total_bytes}")
    print(f"  Total frames: {total_frames}")
    print(f"  Firmware CRC32: 0x{stm32_crc32(firmware):08X}")

    boot_verify = verify_bootloader_source(bootloader_path)
    print("\nBootloader source check:")
    print(f"  APP1 address correct: {boot_verify['app1_addr']}")
    print(f"  APP2 address correct: {boot_verify['app2_addr']}")
    print(f"  Jump-to-app handler pattern: {boot_verify['jump_handler']}")
    print(f"  MSP set from vector table: {boot_verify['jump_msp']}")
    print(f"  Button selection logic present: {boot_verify['button_logic']}")

    if dump_packet:
        dump_path = firmware_path.parent / "ota_start_packet.bin"
        dump_path.write_bytes(start_packet)
        print(f"\nDumped start packet to: {dump_path}")

    print("\nOTA packet generation and bootloader static checks are complete.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate OTA packet generation and bootloader source logic.")
    parser.add_argument("firmware", type=Path, default=Path("blink.bin"), nargs="?",
                        help="Path to the firmware bin to use for OTA testing")
    parser.add_argument("--bootloader", type=Path, default=Path("Bootloader/Src/main.c"),
                        help="Path to the bootloader main source file")
    parser.add_argument("--dump", action="store_true", help="Dump the OTA start packet to a binary file")
    args = parser.parse_args()
    if not args.firmware.exists():
        print(f"Firmware file not found: {args.firmware}")
        return 1
    if not args.bootloader.exists():
        print(f"Bootloader source file not found: {args.bootloader}")
        return 1
    return report(args.firmware, args.bootloader, dump_packet=args.dump)


if __name__ == "__main__":
    raise SystemExit(main())

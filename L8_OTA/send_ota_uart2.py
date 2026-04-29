#!/usr/bin/env python3
import argparse
import struct
import sys
from pathlib import Path

try:
    import serial
except ImportError:
    serial = None

OTA_MAGIC = 0xDEADBEEF
HEADER_SIZE = 12


def stm32_crc32(data: bytes) -> int:
    """STM32 CRC peripheral default: poly 0x04C11DB7, init 0xFFFFFFFF,
    no reflection, no final xor."""
    crc = 0xFFFFFFFF
    poly = 0x04C11DB7

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


def build_packet(payload: bytes) -> bytes:
    if len(payload) > 0xFFFF:
        raise ValueError("Firmware too large for 16-bit length field")

    crc = stm32_crc32(payload)
    header = struct.pack("<IHHI", OTA_MAGIC, len(payload), 0, crc)
    return header + payload


def open_serial(port: str, baud: int):
    if serial is None:
        raise RuntimeError("pyserial is not installed. Run: pip install pyserial")
    return serial.Serial(port=port, baudrate=baud, bytesize=8, parity='N', stopbits=1, timeout=5)


def main() -> int:
    parser = argparse.ArgumentParser(description="Send blink.bin to STM32 via UART2 OTA packet")
    parser.add_argument("firmware", type=Path, help="Path to firmware binary, e.g. L8_OTA/blink.bin")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="Serial port for UART2 adapter")
    parser.add_argument("--baud", type=int, default=9600, help="UART baud rate")
    parser.add_argument("--dry-run", action="store_true", help="Print packet info without sending")
    args = parser.parse_args()

    firmware = args.firmware.read_bytes()
    packet = build_packet(firmware)
    crc = stm32_crc32(firmware)

    print(f"Firmware: {args.firmware}")
    print(f"Size: {len(firmware)} bytes")
    print(f"CRC32 (STM32 style): 0x{crc:08X}")
    print(f"Packet size: {len(packet)} bytes")

    if args.dry_run:
        return 0

    ser = open_serial(args.port, args.baud)
    with ser:
        # Give the board a moment to print its ready prompt before sending.
        ser.write(packet)
        ser.flush()

    print("Packet sent.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

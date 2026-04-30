import argparse
import struct
import sys
import time
from pathlib import Path

try:
    import serial
except ImportError:
    serial = None

OTA_HEADER = 0x00A5
OTA_ACK = 0x06
OTA_NACK = 0x15

CMD_START = 0x01
CMD_DATA = 0x02
CMD_END = 0x03

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

def build_packet(seq_num: int, cmd: int, payload: bytes) -> bytes:
    if len(payload) > 128:
        raise ValueError("Payload max 128 bytes")
        
    padded_payload = payload + b"\x00" * (128 - len(payload))
    
    # Pack: Header(2), Seq(2), Cmd(1), Len(1), Payload(128) -> Total 134 bytes
    fmt = "<HHBB128s"
    packet_data = struct.pack(fmt, OTA_HEADER, seq_num, cmd, len(payload), padded_payload)
    
    # Calculate CRC32 on the 134 bytes
    crc = stm32_crc32(packet_data)
    
    # Append CRC32 (4 bytes) -> Total 138 bytes
    return packet_data + struct.pack("<I", crc)

def open_serial(port: str, baud: int):
    if serial is None:
        raise RuntimeError("pyserial is not installed. Run: pip install pyserial")
    return serial.Serial(port=port, baudrate=baud, bytesize=8, parity='N', stopbits=1, timeout=5)

def wait_for_ack(ser, expected_seq: int) -> bool:
    # Read response (9 bytes: Header(2), Seq(2), Cmd(1), CRC32(4))
    res = ser.read(9)
    if len(res) != 9:
        print(f"Error: Timeout waiting for response, received {len(res)} bytes")
        return False
        
    header, seq, cmd, crc = struct.unpack("<HHBI", res)
    if header != OTA_HEADER:
        print(f"Error: Invalid response header: 0x{header:04X}")
        return False
        
    if cmd == OTA_NACK:
        print(f"Error: Target responded with NACK for seq {seq}. MCU calculated CRC: 0x{crc:08X}")
        return False
        
    if cmd != OTA_ACK:
        print(f"Error: Target responded with unknown cmd: {cmd}")
        return False
        
    if seq != expected_seq:
        print(f"Error: Seq mismatch, expected {expected_seq}, got {seq}")
        return False
        
    return True

def main() -> int:
    parser = argparse.ArgumentParser(description="Send firmware to STM32 via UART2 OTA packet")
    parser.add_argument("firmware", type=Path, help="Path to firmware binary, e.g. blink.bin")
    # Change default port to COM1 or COM3 depending on typical Windows setups, but we'll use COM1
    parser.add_argument("--port", default="COM18", help="Serial port for UART2 adapter (e.g. COM1, COM3, /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=9600, help="UART baud rate")
    parser.add_argument("--dry-run", action="store_true", help="Print packet info without sending")
    args = parser.parse_args()

    firmware = args.firmware.read_bytes()
    global_crc = stm32_crc32(firmware)
    total_bytes = len(firmware)
    total_frames = (total_bytes + 127) // 128
    
    print(f"Firmware: {args.firmware}")
    print(f"Size: {total_bytes} bytes")
    print(f"Total Frames: {total_frames}")
    print(f"Global CRC32: 0x{global_crc:08X}")

    if args.dry_run:
        return 0

    try:
        ser = open_serial(args.port, args.baud)
    except serial.serialutil.SerialException as e:
        print(f"Failed to open port {args.port}: {e}")
        print("Please check your serial port name. On Windows it's usually COM1, COM3, etc.")
        return 1
        
    seq_num = 0
    with ser:
        print("\n--- Sending OTA_START ---")
        # Start Payload: Version(4), TotalBytes(4), TotalFrames(2), GlobalCRC(4)
        version = 1
        start_payload = struct.pack("<IIHI", version, total_bytes, total_frames, global_crc)
        start_packet = build_packet(seq_num, CMD_START, start_payload)
        
        ser.write(start_packet)
        ser.flush()
        if not wait_for_ack(ser, seq_num):
            return 1
            
        seq_num += 1
        print("OTA_START Acknowledged.")

        print("\n--- Sending OTA_DATA ---")
        for i in range(total_frames):
            chunk = firmware[i*128 : (i+1)*128]
            packet = build_packet(seq_num, CMD_DATA, chunk)
            ser.write(packet)
            ser.flush()
            
            # Simple progress bar
            print(f"\rSending frame {i+1}/{total_frames}...", end="")
            
            if not wait_for_ack(ser, seq_num):
                print(f"\nFailed at frame {i+1}")
                return 1
            seq_num += 1
            
        print("\nOTA_DATA transmission complete.")

        print("\n--- Sending OTA_END ---")
        end_packet = build_packet(seq_num, CMD_END, b"")
        ser.write(end_packet)
        ser.flush()
        if not wait_for_ack(ser, seq_num):
            return 1
            
        print("OTA_END Acknowledged. Update successful!")

    return 0

if __name__ == "__main__":
    sys.exit(main())

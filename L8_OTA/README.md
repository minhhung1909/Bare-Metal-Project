# L8_OTA — Hệ thống Bootloader và OTA qua UART (A/B Ping-Pong)

Thư mục này chứa kiến trúc phân vùng OTA hoàn chỉnh cho STM32G431. Mục tiêu là cho phép thiết bị cập nhật phần mềm từ xa thông qua giao tiếp UART mà không cần mạch nạp vật lý.

## 1) Tổng quan kiến trúc hệ thống

Dự án chia vùng Flash thành 3 phần chính, tương ứng với 3 App độc lập:
1. **Bootloader (`Bootloader/`)**: Quản lý quyết định boot. Đọc thông tin từ vùng nhớ Shared Metadata để quyết định nhảy vào (Jump) App1 hay App2.
2. **Application (`Application/`)**: Ứng dụng nhận nhiệm vụ đón dữ liệu OTA. Chứa logic nhận các gói tin qua UART2 bằng DMA, tính toán CRC, xóa Flash và ghi firmware mới vào phân vùng an toàn, sau đó cập nhật Metadata và yêu cầu MCU reset.
3. **UART (`UART/`)**: Ứng dụng demo để kiểm thử OTA. Nhiệm vụ của nó là chớp nháy LED và liên tục in/nhận log qua UART mỗi 1 giây (`Hello World`). Ứng dụng này sẽ được compile ra `.bin` và được bắn OTA xuống MCU để chạy ở phân vùng thứ 2 (App 2).

> **Bản chất Ping-Pong**: Quá trình cập nhật luôn diễn ra trên một phân vùng an toàn (chưa chạy). Ví dụ MCU đang chạy ở App 1, khi nhận OTA nó sẽ ghi firmware mới sang App 2 và ngược lại, bảo vệ mạch khỏi nguy cơ bị chết (brick) giữa chừng.

## 2) Phân chia địa chỉ bộ nhớ Flash

Bộ nhớ Flash (tổng quan 128KB) được quy hoạch như sau:
- **Bootloader**: `0x08000000` — `0x08003FFF` (16KB)
- **App slot 1 (Application)**: `0x08004000` — `0x08011FFF` (56KB)
- **App slot 2 (UART App)**: `0x08012000` — `0x0801E7FF` (50KB)
- **Metadata/Shared OTA page**: `0x0801E800` (Page 61) — Chứa `OTA_Shared_Config_t` (Magic Word, Active App).

## 3) Luồng OTA hoạt động (OTA Flow)

1. Thiết bị đang chạy ở **App 1** (Application).
2. Người dùng sử dụng script Python `send_ota_uart2.py` để gửi file `uart.bin` (của App 2) qua UART.
3. **App 1** nhận gói tin OTA `OTA_START`, tiến hành xóa phân vùng của App 2 (`0x08012000`) để dọn đường.
4. Host tiếp tục bắn các gói `OTA_DATA` chứa firmware. **App 1** ghi dần vào phân vùng App 2.
5. Sau khi nhận lệnh `OTA_END`, MCU tính lại CRC32 toàn vẹn của phân vùng App 2 trên Flash và so sánh với CRC gốc được báo từ `OTA_START`.
6. Nếu hợp lệ, **App 1** cập nhật vùng Shared Metadata (báo cho Bootloader biết App 2 sẽ là App active) và khởi động lại chip.
7. **Bootloader** khởi động, đọc Shared Metadata, thấy App 2 được trỏ tới liền cấu hình Vector Table (VTOR) và nhảy sang `0x08012000`.
8. **App UART** bắt đầu chạy, nháy LED và truyền log. Hoàn thành OTA!

## 4) Định dạng gói OTA (Frame)

Việc trao đổi diễn ra đồng bộ, Host gửi 1 gói, MCU trả lời ACK/NACK (9 bytes). Gói gửi từ Host -> MCU có kích thước chính xác **138 bytes**:

- `Header` (2 bytes): `0x00A5`
- `Seq_num` (2 bytes): Số thứ tự gói
- `CommandId` (1 byte): `0x01` (START), `0x02` (DATA), `0x03` (END)
- `Length` (1 byte): Kích thước Payload thực tế (tối đa 128)
- `Payload` (128 bytes): Dữ liệu Firmware được chia nhỏ
- `CRC32` (4 bytes): Mã kiểm tra lỗi phần cứng

## 5) Hướng dẫn Test Nhanh bằng `send_ota_uart2.py`

### Bước 1: Build các Project
Đi vào từng thư mục và build ra file nhị phân (`.bin`):
```bash
(cd Bootloader && make clean && make)
(cd Application && make clean && make)
(cd UART && make clean && make)
```

### Bước 2: Nạp Bootloader và App1
Sử dụng ST-Link để nạp lần lượt:
1. Nạp Bootloader vào `0x08000000`
2. Nạp Application vào `0x08004000`

### Bước 3: Gửi firmware OTA
Lấy file `uart.bin` (kết quả của project UART) và copy ra ngoài, sử dụng Python để đẩy qua UART:
```powershell
python3 .\send_ota_uart2.py .\uart.bin --port COMx --baud 115200
```
> *Lưu ý: Thay `COMx` bằng cổng Serial kết nối đến vi điều khiển của bạn (VD: `COM1`, `/dev/ttyUSB0`).*

Sau khi chạy lệnh, Python sẽ truyền các Frame OTA, MCU sẽ báo % hoàn thành. Nếu thành công, vi điều khiển sẽ khởi động lại, chuyển sang App UART và LED sẽ chớp nháy.

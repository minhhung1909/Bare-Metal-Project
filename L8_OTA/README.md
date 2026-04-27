- 



# L8_OTA - Khung Bootloader + App1 + App2

Thư mục này là bước chuyển tiếp từ bài flash đơn lẻ sang kiến trúc OTA. Hiện tại repo đã có khung 3 project tách rời nhưng chưa hoàn thiện luồng OTA đầy đủ.

## 1) Cấu trúc hiện tại

- `Bootloader/`: project bootloader (độc lập Makefile/linker/startup).
- `App1/`: project ứng dụng slot 1.
- `App2/`: project ứng dụng slot 2.

Mỗi project đều tự build ra `build/firmware.elf` và `build/firmware.bin`.

## 2) Trạng thái code thực tế (quan trọng)

### 2.1 Bootloader

`Bootloader/Src/main.c` hiện đang giống bài L7_Flash:

- Chuẩn bị buffer chuỗi.
- Erase page 48.
- Program vào `0x08018000`.
- Chớp LED trong vòng lặp vô hạn.

Chưa có các phần cốt lõi của bootloader OTA như:

- Kiểm tra metadata/CRC image.
- Chọn App1/App2 để boot.
- Đổi `MSP` và nhảy tới reset handler của app.
- Cơ chế rollback khi app lỗi.

### 2.2 App1 và App2

`App1` và `App2` hiện đang là app mẫu blink LED (SysTick + PC6), chưa có giao tiếp với bootloader.

### 2.3 Linker map hiện tại

- `Bootloader/g431.ld`: flash `0x08000000`, length `16k`.
- `App1/g431.ld`: flash `0x08000000`, length `56k`.
- `App2/g431.ld`: flash `0x08000000`, length `56k`.

Như vậy App1 và App2 đang trùng vùng flash với nhau (và trùng base với bootloader), nên chưa phải map OTA hoàn chỉnh.

## 3) Build từng project

Từ thư mục `L8_OTA`:

```bash
(cd Bootloader && make clean && make)
(cd App1 && make clean && make)
(cd App2 && make clean && make)
```

## 4) Flash thử nghiệm (trạng thái hiện tại)

Mỗi project đang có target `make flash` ghi vào `0x08000000`.

Điều này có nghĩa là:

- Flash project sau sẽ ghi đè project trước.
- Không thể cùng lúc giữ bootloader + app theo đúng mô hình OTA.

## 5) Đề xuất map flash để hoàn thiện OTA

Ví dụ với STM32G431 128 KB (64 page, mỗi page 2 KB):

1. Bootloader: `0x08000000` - `0x08003FFF` (16 KB, page 0..7)
2. App1 slot: `0x08004000` - `0x0800BFFF` (32 KB, page 8..23)
3. App2 slot: `0x0800C000` - `0x08013FFF` (32 KB, page 24..39)
4. Metadata/flags: phần còn lại (ví dụ page cuối)

Sau đó cần sửa đồng bộ:

- Linker script của `App1` và `App2` (ORIGIN khác nhau).
- Địa chỉ `st-flash write` cho từng project.
- Bootloader logic kiểm tra vector table app và jump đúng slot.

## 6) Checklist để biến L8 thành OTA thật

1. Chốt phân vùng flash (boot + 2 slot + metadata).
2. Viết cấu trúc metadata (version, size, CRC, trạng thái valid/pending).
3. Viết hàm jump app chuẩn (set MSP, set VTOR, branch reset handler).
4. Thêm cơ chế xác thực image trước khi boot.
5. Thêm rollback nếu app mới không xác nhận healthy.
6. Tách luồng update UART/CAN/USB và luồng boot.

## 7) Ghi nhớ nhanh

1. L8 hiện tại là skeleton nhiều project, chưa phải OTA hoàn chỉnh.
2. Bootloader đang mới dừng ở thao tác flash demo.
3. Muốn OTA đúng nghĩa thì bắt buộc phải tách ORIGIN của App1/App2.
4. Map bộ nhớ là bước đầu tiên cần chốt trước khi viết logic boot.
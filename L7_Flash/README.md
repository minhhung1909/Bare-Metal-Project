# L7_Flash - Ghi/đọc Flash nội trên STM32G431

Bài này tập trung vào thao tác flash nội bằng thanh ghi: xóa 1 page và ghi dữ liệu theo đơn vị double-word (8 byte).

## 1) Mục tiêu bài học

- Hiểu quy trình mở khóa flash, xóa page, ghi dữ liệu.
- Làm đúng thứ tự cờ trạng thái `BSY`, `EOP`, và các cờ lỗi.
- Nắm quy tắc căn lề 8 byte khi program flash.
- Tạo nền cho các bài bootloader/OTA ở bước sau.

## 2) Trạng thái code hiện tại

Trong `main()`:

1. Tạo chuỗi `"Hello World Le Minh Hung \r\n"`.
2. Chuẩn bị `flash_buffer[30]` (căn 8 byte) với phần dư được đệm `0xFF`.
3. Khởi tạo SysTick, UART, LED.
4. Xóa page 48.
5. Ghi dữ liệu từ `flash_buffer` vào địa chỉ `0x08018000`.
6. Vòng lặp vô hạn chỉ chớp LED mỗi 500 ms.

## 3) Sơ đồ vùng nhớ flash liên quan

Theo linker `g431.ld` của bài này:

- Flash toàn chip: 128 KB từ `0x08000000`.
- STM32G431 có 64 page, nên mỗi page là 2 KB.

Địa chỉ đầu page được tính:

$$Address(page\ n) = 0x08000000 + n \times 0x800$$

Với page 48:

$$0x08000000 + 48 \times 0x800 = 0x08018000$$

Đây chính là địa chỉ code đang ghi.

## 4) Phân tích hàm xóa page

Hàm `ErasePage(uint8_t page)` trong `Src/flash.c` đang làm đúng các bước cốt lõi:

1. Kiểm tra phạm vi page (`0..63`).
2. Chờ `FLASH_SR.BSY` về 0.
3. Nếu flash đang khóa (`FLASH_CR.LOCK`), ghi 2 key mở khóa:
	- `0x45670123`
	- `0xCDEF89AB`
4. Xóa nhóm cờ lỗi bằng cách ghi 1 vào các bit lỗi (`FLASH_SR & 0x3F8U`).
5. Ghi số page vào trường `PNB` (bit 8:3) và bật `PER` (bit 1).
6. Set `STRT` (bit 16) để trigger xóa.
7. Chờ `BSY` về 0.
8. Tắt lại `PER`.

## 5) Phân tích hàm program flash

Hàm `Program_Flash(void* addr, uint64_t* data_array, uint16_t size)` đang ghi theo đơn vị 8 byte:

1. Chờ `BSY` và xóa cờ lỗi trước khi ghi.
2. Bật `FLASH_CR.PG`.
3. Với mỗi phần tử `uint64_t`:
	- Tách `low 32-bit` và `high 32-bit`.
	- Ghi lần lượt 2 từ 32-bit vào flash liên tiếp.
	- Chờ `BSY` clear.
	- Nếu `EOP` set thì ghi 1 để clear.
4. Tắt `PG` sau khi xong.

Trong `main()`, số double-word được tính an toàn:

```c
uint16_t double_words_count = (sizeof(flash_buffer) + 7) / 8;
```

Với `flash_buffer[30]`, kết quả là 4 double-word (32 byte), phần dư đã đệm `0xFF`.

## 6) Build và flash

Trong thư mục `L7_Flash`:

```bash
make clean
make
make flash
```

## 7) Cách verify dữ liệu đã ghi

### 7.1 Kiểm tra bằng st-flash readback

Đọc 64 byte từ địa chỉ vừa ghi:

```bash
st-flash read dump_l7.bin 0x08018000 64
xxd dump_l7.bin
```

Bạn sẽ thấy chuỗi `Hello World Le Minh Hung` và phần còn lại là `FF`.

### 7.2 Kiểm tra bằng GDB

Sau khi kết nối target trong GDB:

```gdb
x/32bx 0x08018000
x/s 0x08018000
```

## 8) Lỗi hay gặp khi làm flash

- Ghi vào flash khi chưa erase page trước đó.
- Địa chỉ không căn 8 byte cho chế độ program double-word.
- Quên chờ `BSY` trước/sau thao tác ghi-xóa.
- Không clear cờ lỗi cũ làm lần ghi sau thất bại.
- Xóa nhầm page đang chứa firmware đang chạy.

## 9) Ghi nhớ nhanh

1. Erase theo page, program theo double-word (8 byte).
2. Địa chỉ ghi phải đúng căn lề và đúng page mục tiêu.
3. `BSY` luôn phải được theo dõi trước/sau thao tác.
4. Xóa cờ lỗi và `EOP` đúng cách để lần thao tác kế tiếp ổn định.

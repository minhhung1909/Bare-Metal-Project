# L5_UART - Ghi chú học lại

Tài liệu này viết để sau này mở lại có thể nhớ nhanh bài UART đang làm ở mức bare-metal.

## 1) Mục tiêu bài này

- Cấu hình USART2 bằng thanh ghi (không dùng HAL).
- Gửi dữ liệu UART theo 2 kiểu:
  - Gửi 1 ký tự: `UART_Transmit()`
  - Gửi nhiều ký tự: `UART_Transmit_multi()`
- Nhận dữ liệu theo 2 kiểu:
  - Polling: `UART_Receiver()`
  - Ngắt RXNE: `UART_INIT_IVIC()` + `UART2_Interrup_Handler()`
- Dùng SysTick để tạo `delay_ms()` và nháy LED để biết chương trình đang chạy.

## 2) Tổng quan luồng chương trình

Trong `main()`:

1. `sysTick_Init(SYSCLK_HZ)` khởi tạo tick 1 ms.
2. Chọn 1 trong 2 cách khởi tạo UART:
   - `uart_init()` nếu dùng polling
   - `UART_INIT_IVIC()` nếu dùng ngắt RX
3. Khởi tạo LED PC6, gửi chuỗi "Hello World".
4. Vào vòng lặp vô hạn, LED đảo trạng thái mỗi 500 ms.
5. Nếu có ngắt UART RXNE, ISR sẽ đọc `USART2_RDR` và cất vào mảng `data_recei[]`.

## 3) Cấu hình UART2 trong bài này

### 3.1 Chân và clock

- UART dùng: USART2
- Chân:
  - PA2 = TX
  - PA3 = RX
- Bật clock:
  - `RCC_AHB2ENR` bit 0: cấp clock cho GPIOA
  - `RCC_APB1ENR1` bit 17: cấp clock cho USART2

### 3.2 GPIO alternate function

- `GPIOA_MODER`:
  - PA2, PA3 đặt mode Alternate Function (`10`)
- `GPIOA_AFRL`:
  - AF7 cho PA2 và PA3 để map sang USART2

### 3.3 Baudrate

Code đang dùng:

- `APB1_CLOCK = 16000000`
- `BAUDRATE = 9600`
- `USART2_BRR = APB1_CLOCK / BAUDRATE`

Giá trị BRR tính ra xấp xỉ 1666 (0x682).

### 3.4 Bật TX/RX/UART

Trong `USART2_CR1`:

- Bit 0 (`UE`) = 1: enable USART
- Bit 2 (`RE`) = 1: enable receiver
- Bit 3 (`TE`) = 1: enable transmitter

Nếu dùng ngắt RX:

- Bit 5 (`RXNEIE`) = 1: cho phép ngắt khi RX data register not empty

### 3.5 NVIC cho USART2

- IRQ number của USART2 là 38.
- NVIC ISER1 quản lý IRQ từ 32..63.
- Vì vậy bật bit `38 - 32 = 6` trong `NVIC_ISER1`.

Code đang dùng:

```c
NVIC_ISER1 |= (1U << 6);
```

## 4) Các hàm quan trọng

### `uart_init()`

Khởi tạo UART theo polling (không bật RXNE interrupt).

### `UART_INIT_IVIC()`

Khởi tạo UART có bật RXNE interrupt + bật NVIC.

Lưu ý: tên hàm hiện tại là `IVIC` (theo code hiện tại), dự ý định là NVIC.

### `UART_Transmit(char data)`

- Chờ `TXE` (ISR bit 7) = 1 rồi mới ghi `USART2_TDR`
- Chờ `TC` (ISR bit 6) = 1 để đảm bảo frame gửi xong

### `UART_Transmit_multi(char* arr, uint8_t size)`

- Lặp qua mảng, mỗi byte đều chờ `TXE`
- Cuối cùng chờ `TC` để chắc chắn gửi xong toàn bộ

### `UART_Receiver()`

- Chờ `RXNE` (ISR bit 5) = 1
- Đọc `USART2_RDR`

### `UART2_Interrup_Handler()`

- Mỗi lần ngắt RXNE, đọc 1 byte từ `USART2_RDR`
- Cất vào `data_recei[rx_index]`, sau đó `rx_index++`

## 5) Liên kết vector ngắt

Trong `startup_g4.c`, vector USART2 được gán như sau:

```c
[16+38] = UART2_Interrup_Handler
```

Nghĩa là hàm ISR trong `main.c` phải trùng tên `UART2_Interrup_Handler` như đang viết.

## 6) Cách build và flash

Trong thư mục `L5_Uart_NVIC`:

```bash
make
make flash
```

Nếu dùng WSL2 + ST-Link, tham khảo hướng dẫn attach USB ở README tổng của repo.

## 7) Cách test nhanh UART

1. Flash firmware.
2. Mở serial terminal đúng tốc độ 9600 baud.
3. Sau reset, board gửi chuỗi: `Hello World`.
4. Nếu gõ ký tự vào serial:
   - polling: phải tự gọi `UART_Receiver()` trong loop
   - interrupt: ISR sẽ tự nhận và cất vào `data_recei[]`

Gợi ý khi test:

- Nhớ chung GND giữa board và USB-UART nếu dùng module rời.
- Kiểm tra đúng cổng serial (`/dev/ttyACM*` hoặc `/dev/ttyUSB*`).

## 8) Các lỗi hay gặp

- Quên bật clock GPIOA/USART2.
- Set sai Alternate Function (không phải AF7).
- Sai baudrate do nhầm clock APB1.
- Bật `RXNEIE` nhưng quên bật NVIC.
- Đang dùng ngắt nhưng tên ISR không đúng với vector table.
- `data_recei[]` chưa có check tràn mảng (nếu gõ quá nhiều ký tự sẽ tràn).

## 9) Việc nên nâng cấp tiếp

- Thêm giới hạn `rx_index` để tránh tràn bộ đệm.
- Thêm cơ chế kết thúc chuỗi (`'\0'`) để in/debug dễ hơn.
- Xử lý lỗi UART (ORE, FE, NE) trong ISR.
- Chuyển sang DMA ở bài L6 để nhận/gửi khối dữ liệu lớn hiệu quả hơn.

---

Nếu mở lại bài này để ôn nhanh, chỉ cần nhớ 4 bước:

1. Bật clock GPIOA + USART2.
2. Đặt PA2/PA3 = AF7.
3. Đặt BRR và bật UE/TE/RE.
4. Nếu dùng ngắt: bật RXNEIE + NVIC + ISR đúng tên trong startup.
# L6_UART_DMA - Ghi chú học lại

L6 đã chuyển nhận UART sang DMA và hiện tại chạy theo kiểu nhận vòng (circular) trên DMA1 Channel 1.

## 1) Mục tiêu bài này

- Chuyển RX của USART2 từ polling/ngắt RXNE sang DMA.
- Dùng DMAMUX để route request USART2_RX vào DMA1 Channel 1.
- Cấu hình DMA theo hướng Peripheral-to-Memory.
- Bật ngắt Transfer Complete để biết mỗi lần nhận đầy buffer.
- Bật Circular mode để nhận liên tục, không cần re-arm thủ công sau mỗi frame cố định.

## 2) Trạng thái code hiện tại

Trong `main()`:

1. Khởi tạo SysTick 1 ms.
2. Gọi `uart_init()`.
3. Gọi `DMA_init(data_recei, sizeof(data_recei))`.
4. Gửi `Hello World` một lần.
5. Vòng lặp vô hạn chỉ nháy LED PC6 mỗi 500 ms.

Lưu ý:

- Buffer RX hiện tại: `data_recei[20]`.
- DMA đang ở chế độ circular nên dữ liệu mới sẽ tiếp tục ghi đè lên buffer theo vòng.
- ISR DMA mới chỉ xóa cờ ngắt, chưa có logic parse/echo dữ liệu.

## 3) Luồng dữ liệu RX hiện tại

`PC gửi UART` -> `USART2_RDR` -> `DMAMUX (REQ ID 26)` -> `DMA1 Channel 1` -> `data_recei[]`

CPU không cần đọc từng byte từ `RDR`. CPU chỉ cần xử lý ở mức sự kiện (ví dụ khi TC interrupt xảy ra).

## 4) Cấu hình UART2 liên quan DMA

Hàm `uart_init()` đang cấu hình:

- Bật clock GPIOA (`RCC_AHB2ENR` bit 0).
- PA2/PA3 sang Alternate Function.
- AF7 cho PA2/PA3 (`GPIOA_AFRL`).
- Bật clock USART2 (`RCC_APB1ENR1` bit 17).
- Baud 9600 (`USART2_BRR = 16000000/9600`).
- Bật `UE`, `RE`, `TE` trong `USART2_CR1`.

Trong `DMA_init()`, bật `USART2_CR3` bit `DMAR` để cho phép RX đi qua DMA.

## 5) Cấu hình DMAMUX + DMA

### 5.1 Bật clock khối DMA

- `RCC_AHB1ENR` bit 0: DMA1.
- `RCC_AHB1ENR` bit 2: DMAMUX.

### 5.2 Chọn nguồn request

- `DMAMUX_C0CR` chọn request ID `26` (USART2_RX).

### 5.3 Gán địa chỉ và độ dài

- `DMA_CPAR1 = &USART2_RDR`.
- `DMA_CMAR1 = data_recei`.
- `DMA_CNDTR1 = size`.

### 5.4 Cấu hình `DMA_CCR1`

Code hiện tại bật:

- Bit 7 (`MINC`) = 1: tự tăng địa chỉ RAM.
- Bit 5 (`CIRC`) = 1: circular mode.
- Bit 1 (`TCIE`) = 1: ngắt khi đầy buffer.
- Bit 0 (`EN`) = 1: enable channel.

Mặc định (không set) nên vẫn đúng cho bài này:

- `DIR` = 0: Peripheral-to-Memory.
- `MSIZE/PSIZE` = `00`: 8-bit (phù hợp mảng `char`).

## 6) Ngắt DMA Channel 1

### 6.1 NVIC

- `NVIC_ISER0` bit 11 được bật trong `DMA_init()`.

### 6.2 Vector table

Trong `startup_g4.c`:

```c
[16+11] = DMA_Channel_1_IQRHandler
```

Lưu ý tên ISR đang là `IQRHandler` (theo code hiện tại), phải giữ đúng tên để map vector chính xác.

### 6.3 ISR hiện tại

```c
void DMA_Channel_1_IQRHandler(void)
{
    DMA1_IFCR |= (1U << 1) | (1U << 0);
    __asm("NOP");
}
```

Ý nghĩa:

- Xóa cờ global + transfer complete của DMA Channel 1.
- Chưa xử lý dữ liệu ở tầng ứng dụng.

## 7) Hành vi circular cần nắm

- Sau mỗi lần nhận đủ 20 byte, DMA phát TC interrupt.
- Khi `CIRC = 1`, phần cứng tự nạp lại bộ đếm và tiếp tục chạy vòng mới.
- Buffer `data_recei[]` sẽ bị ghi đè theo chu kỳ 20 byte/lần.

## 8) Build và flash

Trong thư mục `L6_Uart_DMA`:

```bash
make
make flash
```

## 9) Cách test nhanh với circular RX

1. Flash firmware.
2. Mở terminal serial ở 9600 baud.
3. Reset board, xác nhận có chuỗi `Hello World`.
4. Gửi liên tục nhiều hơn 20 byte (ví dụ 40-60 byte).
5. Đặt breakpoint ở `DMA_Channel_1_IQRHandler`, sẽ thấy ngắt lặp lại theo chu kỳ đầy buffer.
6. Trong GDB quan sát `data_recei` để thấy dữ liệu mới ghi đè theo vòng.

Gợi ý lệnh GDB:

```gdb
p data_recei
p/x DMA_CNDTR1
```

## 10) So sánh nhanh L5 và L6

- L5: CPU xử lý byte-by-byte khi RXNE.
- L6: DMA tự chuyển byte vào RAM; CPU xử lý theo block/sự kiện.
- Circular mode của L6 phù hợp stream liên tục vì không cần re-arm thủ công mỗi lần đầy buffer.

## 11) Việc nên làm tiếp

1. Thêm chiến lược đọc buffer an toàn để tránh race giữa CPU và DMA.
2. Bật thêm Half Transfer interrupt để xử lý theo nửa buffer.
3. Kết hợp USART IDLE line để đóng gói frame độ dài biến thiên.
4. Bổ sung xử lý lỗi UART (`ORE`, `FE`, `NE`).

## 12) Ghi nhớ nhanh

1. Bật clock DMA1 + DMAMUX.
2. DMAMUX Channel 0 chọn request USART2_RX (ID 26).
3. Set `CPAR = USART2_RDR`, `CMAR = buffer`, `CNDTR = size`.
4. Bật `MINC + CIRC + TCIE + EN` trong `DMA_CCR1`.
5. Bật `USART2_CR3.DMAR` + bật NVIC IRQ DMA Channel 1.

# Hướng dẫn Debug STM32G431 bằng OpenOCD + GDB

Tài liệu này dùng cho toàn bộ project Bare-Metal trong repo, đặc biệt hữu ích khi debug các bài TIM6/TIM1/UART.

## 1) Mô hình hoạt động

GDB không nói chuyện trực tiếp với chip. Luôn theo mô hình Client - Server:

- Server: `openocd` giao tiếp với ST-Link và MCU qua SWD/JTAG, mở cổng `3333`.
- Client: `arm-none-eabi-gdb` nạp file `.elf`, kết nối đến `localhost:3333` để điều khiển CPU.

## 2) Checklist trước khi debug

1. Build có debug symbol:
  - `-g3`
  - `-O0`
2. Đã có file `.elf` của bài cần debug.
3. ST-Link đã attach vào WSL2 (nếu chạy trên Windows + WSL2).
4. Có sẵn các lệnh:
  - `openocd`
  - `arm-none-eabi-gdb`
  - `st-flash`

## 3) Quy trình debug chuẩn

### Bước 1: Build firmware

Ví dụ debug bài UART:

```bash
cd L5_Uart_NVIC
make
```

### Bước 2: Mở OpenOCD ở Terminal 1

```bash
openocd -f interface/stlink.cfg -f target/stm32g4x.cfg
```

Giữ terminal này chạy trong suốt phiên debug.

### Bước 3: Mở GDB ở Terminal 2

Nếu đang đứng trong thư mục bài:

```bash
arm-none-eabi-gdb build/firmware.elf
```

Nếu đang đứng ở root repo:

```bash
arm-none-eabi-gdb L5_Uart_NVIC/build/firmware.elf
```

### Bước 4: Kết nối chip trong dấu nhắc `(gdb)`

```gdb
target extended-remote localhost:3333
monitor reset halt
load
thbreak main
continue
```

Ý nghĩa nhanh:

- `target extended-remote ...`: kết nối OpenOCD.
- `monitor reset halt`: reset và dừng CPU.
- `load`: nạp firmware mới vào Flash.
- `thbreak main`: đặt breakpoint tạm tại `main`.
- `continue`: chạy chương trình.

## 4) Lệnh GDB dùng nhiều nhất

### 4.1 Điều khiển luồng chạy

- `b main` hoặc `b file.c:line`: đặt breakpoint.
- `c`: chạy tiếp.
- `n`: next (không chui vào hàm).
- `s`: step (chui vào hàm).
- `finish`: chạy đến khi thoát hàm hiện tại.
- `Ctrl + C`: dừng khẩn cấp khi chương trình đang chạy.

### 4.2 Xem biến và thanh ghi lõi

- `p var`: in biến.
- `p/x var`: in hex.
- `p/t var`: in nhị phân.
- `info locals`: xem biến local.
- `info reg`: xem thanh ghi lõi ARM (R0..R15, PC, SP, LR, xPSR).

### 4.3 Soi thanh ghi ngoại vi bằng lệnh `x`

Cú pháp phổ biến:

```gdb
x/1wx <address>
```

Ví dụ:

```gdb
x/1wx 0x40012C34   # TIM1_CCR1
x/1wx 0x40021058   # RCC_APB1ENR1
x/1wx 0x40004400   # USART2_CR1
x/1wx 0xE000E104   # NVIC_ISER1
```

### 4.4 Backtrace khi crash

- `bt`: xem call stack trước khi lỗi.
- `frame <n>`: chuyển frame để soi biến ở từng tầng gọi hàm.

## 5) Checklist debug ngắt theo đúng project

## 5.1 TIM6 (L3/L4)

Kiểm tra lần lượt:

1. Vector table có map đúng ISR:
  - `TIM6_DAC_IRQHandler`
  - slot `16 + 54`
2. NVIC đã bật IRQ 54:
  - `NVIC_ISER1` bit `54 - 32 = 22`
3. TIM6 đã bật interrupt update:
  - `TIM6_DIER` bit `UIE`
4. Trong ISR có xóa cờ `UIF`:
  - nếu không xóa, CPU sẽ kẹt vòng ngắt vô hạn.

## 5.2 USART2 RXNE interrupt (L5)

Kiểm tra lần lượt:

1. Vector table map đúng tên hàm:
  - `UART2_Interrup_Handler`
  - slot `16 + 38`
2. Trong USART2:
  - `UE`, `RE`, `TE` đã bật.
  - `RXNEIE` đã bật (CR1 bit 5).
3. NVIC đã bật IRQ 38:
  - `NVIC_ISER1` bit `38 - 32 = 6`.
4. Trong ISR đã đọc `USART2_RDR` để clear trạng thái RXNE.

## 6) Bộ lệnh debug nhanh theo tình huống

### Tình huống A: Chương trình không vào `main`

```gdb
monitor reset halt
load
b _reset
c
si
```

Nếu không vào `_reset`, kiểm tra lại vector table và địa chỉ flash nạp.

### Tình huống B: Ngắt không chạy

```gdb
b TIM6_DAC_IRQHandler
b UART2_Interrup_Handler
c
```

Nếu không dừng ở breakpoint ISR:

- Soi `NVIC_ISER1`.
- Soi thanh ghi enable interrupt của ngoại vi.
- Kiểm tra tên ISR và vị trí trong vector table.

### Tình huống C: Treo hoặc reset bất thường

```gdb
Ctrl + C
info reg
bt
```

Quan sát `PC`, `LR`, `SP` và call stack để tìm dòng gây lỗi.

## 7) Debug bằng VS Code (Cortex-Debug)

Nếu dùng F5 với extension Cortex-Debug:

1. Cài extension `marus25.cortex-debug`.
2. Kiểm tra `.vscode/launch.json`:
  - Đường dẫn `executable` phải trỏ đến file `.elf` đang debug.
  - Ví dụ đúng cho bài L5 hiện tại:

```json
"executable": "${workspaceFolder}/L5_Uart_NVIC/build/firmware.elf"
```

3. Nếu F5 lỗi parse JSON, mở `launch.json` kiểm tra dấu phẩy và cấu trúc object trước khi debug.

## 8) Dùng ST-Link trong WSL2

Trên PowerShell (Run as Administrator):

```powershell
usbipd list
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>
```

Sau khi attach, quay lại WSL để chạy `openocd` hoặc `make flash`.

## 9) Lỗi thường gặp và cách xử lý

| Hiện tượng | Nguyên nhân hay gặp | Cách xử lý |
|---|---|---|
| `Error: open failed` khi chạy OpenOCD | ST-Link chưa attach vào WSL hoặc thiếu quyền USB | Attach lại bằng `usbipd`, thử chạy với quyền phù hợp |
| GDB báo không có symbol | Build thiếu `-g3` hoặc đang mở sai file `.elf` | Build lại với cờ debug và mở đúng `.elf` |
| Breakpoint không hit | Code tối ưu cao hoặc chưa `load` firmware mới | Dùng `-O0`, chạy lại `monitor reset halt` + `load` |
| ISR không chạy | Sai vector, quên bật NVIC hoặc quên bật interrupt bit ngoại vi | Dùng checklist ở mục 5 để rà từng bước |
| Chip kẹt trong ngắt liên tục | Không xóa cờ ngắt trong ISR | Xóa cờ đúng cách trong ISR trước khi thoát hàm |
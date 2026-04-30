# Bare-Metal STM32G431 Learning Project

Repo này ghi lại quá trình học lập trình nhúng Bare-Metal trên STM32G431 theo hướng "tự làm từ thấp lên cao": tự viết startup, linker script, vector table, cấu hình ngoại vi bằng thanh ghi, build bằng GCC và nạp qua ST-Link.

## 1) Mục tiêu của project

- Hiểu rõ chuỗi khởi động MCU từ reset đến `main()`.
- Làm việc trực tiếp với thanh ghi, không dùng HAL/LL.
- Nắm chắc ngắt (NVIC), timer, PWM, UART, DMA, flash nội.
- Xây nền tảng debug với OpenOCD + GDB để tự xử lý bug firmware.

## 2) Yêu cầu môi trường

### 2.1 Công cụ bắt buộc

- `arm-none-eabi-gcc`
- `arm-none-eabi-gdb`
- `openocd`
- `st-flash` (từ `stlink-tools`)

Toolchain GCC đã có sẵn trong thư mục `install/`.

Ví dụ thêm vào `PATH` (tạm thời trong terminal hiện tại):

```bash
export PATH="$PWD/install/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH"
```

### 2.2 Dùng ST-Link trong WSL2

WSL2 mặc định không nhận USB trực tiếp từ Windows. Cần attach thiết bị bằng `usbipd` ở PowerShell (Run as Administrator):

```powershell
usbipd list
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>
```

Sau đó quay lại WSL để build/flash.

## 3) Cấu trúc chính của repo

- `L1_Blink/`: GPIO cơ bản + delay bận.
- `L2_SysTickTimer/`: SysTick 1 ms + delay ổn định.
- `L3_BasicTimer_INT/`: TIM6 + ngắt định kỳ.
- `L4_AdvancedTimer_PWM/`: TIM1 PWM trên PA8.
- `L5_Uart_NVIC/`: USART2 TX/RX + ngắt RXNE.
- `L6_Uart_DMA/`: USART2 RX bằng DMA + DMAMUX ở circular mode.
- `L7_Flash/`: thao tác flash nội (erase page + program double-word).
- `L8_OTA/`: Khung OTA hoàn chỉnh gồm Bootloader, Application (nhận OTA) và UART App (để test OTA).
- `GDB.md`: tài liệu debug chi tiết bằng GDB.
- `Scheduler.md`: lộ trình học theo tuần.
- `docs/`: tài liệu tham khảo và hình minh họa.

## 4) Danh sách bài học

| Bài | Thư mục | Trọng tâm | Tài liệu |
|---|---|---|---|
| L1 | `L1_Blink` | Bật clock GPIO, cấu hình output, nháy LED | [L1_Blink/README.md](L1_Blink/README.md) |
| L2 | `L2_SysTickTimer` | SysTick interrupt, tạo `delay_ms()` | [L2_SysTickTimer/README.md](L2_SysTickTimer/README.md) |
| L3 | `L3_BasicTimer_INT` | TIM6, cờ update, ISR, NVIC | [L3_BasicTimer_INT/README.md](L3_BasicTimer_INT/README.md) |
| L4 | `L4_AdvancedTimer_PWM` | TIM1 PWM Mode 1, PSC/ARR/CCR, AF | [L4_AdvancedTimer_PWM/README.md](L4_AdvancedTimer_PWM/README.md) |
| L5 | `L5_Uart_NVIC` | USART2 polling + interrupt RXNE | [L5_Uart_NVIC/README.md](L5_Uart_NVIC/README.md) |
| L6 | `L6_Uart_DMA` | USART2 RX bằng DMA circular + TC interrupt | [L6_Uart_DMA/README.md](L6_Uart_DMA/README.md) |
| L7 | `L7_Flash` | Erase/program flash nội bằng thanh ghi | [L7_Flash/README.md](L7_Flash/README.md) |
| L8 | `L8_OTA` | Hệ thống OTA qua UART (Bootloader + Ping Pong App) | [L8_OTA/README.md](L8_OTA/README.md) |

## 5) Build và flash

### 5.1 Build/flash một bài độc lập (L1 -> L7)

Ví dụ với bài flash:

```bash
cd L7_Flash
make
make flash
```

### 5.2 Build nhanh các bài độc lập (L1 -> L7)

```bash
for d in L1_Blink L2_SysTickTimer L3_BasicTimer_INT L4_AdvancedTimer_PWM L5_Uart_NVIC L6_Uart_DMA L7_Flash; do
	echo "==> $d"
	(cd "$d" && make clean && make) || break
done
```

### 5.3 Build cho bài L8 (nhiều subproject)

```bash
(cd L8_OTA/Bootloader && make clean && make)
(cd L8_OTA/Application && make clean && make)
(cd L8_OTA/UART && make clean && make)
```

Với L8, thao tác nạp sẽ tùy theo chiến lược map flash của bootloader/app. Xem chi tiết trong `L8_OTA/README.md`.

## 6) Debug nhanh

Quy trình chuẩn debug:

1. Build ra file `.elf` của bài cần debug.
2. Chạy OpenOCD.
3. Mở GDB, kết nối qua cổng `3333`, `load` firmware, đặt breakpoint.

Tài liệu đầy đủ xem tại [GDB.md](GDB.md).

## 7) Quy ước quan trọng trong project

- Startup và vector table tự viết trong `startup_g4.c`.
- Tên hàm ISR phải khớp tuyệt đối với tên trong vector table.
- Hầu hết bài học đều bật `-g3 -O0` để dễ debug.
- Địa chỉ thanh ghi chủ yếu được gom trong `register.h` (từ các bài sau).

## 8) Lưu ý khi mở bằng VS Code

- Nếu dùng Cortex-Debug, file `.elf` trong `.vscode/launch.json` phải trỏ đúng thư mục bài hiện tại.
- Nếu đổi tên thư mục bài (ví dụ `L5_Uart` -> `L5_Uart_NVIC`), cần cập nhật lại đường dẫn trong `launch.json` để F5 chạy đúng.

## 9) Tài liệu liên quan

- Debug chi tiết: [GDB.md](GDB.md)
- Kế hoạch học: [Scheduler.md](Scheduler.md)
- Linker script docs: <https://sourceware.org/binutils/docs/ld/Scripts.html>
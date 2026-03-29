## L1_Blink – hướng dẫn chi tiết

Ví dụ tối giản: bật clock GPIOC, cấu hình PC6 output, nháy LED bằng vòng lặp bận. Không dùng HAL/RTOS, chỉ dựa trên startup và linker script riêng.

### Prerequisites
- Toolchain: arm-none-eabi-gcc 10.x (đã kèm trong thư mục install/). Thêm vào PATH để dùng nhanh:
	- tạm thời: `export PATH="$PWD/../install/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH"`
	- hoặc chỉnh .bashrc/.zshrc nếu muốn dùng lâu dài.
- Flash tool: st-flash (từ bộ stlink). Trên Ubuntu có thể `sudo apt install stlink-tools`. Cần cắm ST-LINK vào board.
- Windows + WSL: chia sẻ ST-LINK vào WSL qua usbipd.
	- Cài: `winget install usbipd`.
	- Liệt kê USB: `usbipd list`.
	- Gắn vào WSL: `usbipd attach --wsl <Distro Name> --busid <BUSID>` (ví dụ: `usbipd attach --wsl Ubuntu-22.04 --busid 1-4`).
	- Kiểm tra trong WSL: `lsusb` phải thấy thiết bị ST-LINK.

### Cấu trúc thư mục
- Makefile: định nghĩa build (all → firmware.bin) và flash (st-flash về 0x08000000).
- src/main.c: code bật LED PC6, gồm delay bận.
- src/startup_g4.c: vector table + khởi động (nạp SP/PC, clear .bss, copy .data, nhảy vào main, xử lý lỗi).
- linker/g431.ld: phân vùng flash/RAM cho STM32G431.
- build/: sinh sau khi build (elf/bin/map). Bị xoá khi `make clean`.
- Tài liệu linker script: https://sourceware.org/binutils/docs/ld/Scripts.html

### Build
- Đứng tại thư mục L1_Blink:
	- `make` → tạo build/firmware.elf và build/firmware.bin.
	- `make clean` → xoá build/.
- CFLAGS trong Makefile đã bật cảnh báo chặt, optimize O0 để debug, kiến trúc cortex-m4 + hard-fp. Có thể bổ sung cờ riêng bằng `EXTRA_CFLAGS="-O2" make` nếu cần thử tối ưu.

### Flash
- Kết nối ST-LINK, đảm bảo WSL đã thấy thiết bị.
- Chạy: `make flash` (nội bộ dùng `st-flash --reset write build/firmware.bin 0x8000000`). Nếu thiếu quyền USB, thử với sudo hoặc thêm udev rule cho ST-LINK.

### Phần cứng và clock
- MCU: STM32G431 (dòng G4). LED nối PC6 (push-pull). Nếu dùng pin khác, sửa trong src/main.c: ghi lại MODER và bit trong ODR tương ứng.
- Clock: giữ cấu hình reset mặc định của chip. Hàm `delay(5_000_000)` là vòng lặp bận; thời gian phụ thuộc tần số SYSCLK thực tế. Ước lượng:
	- Nếu SYSCLK ~16 MHz, 5e6 vòng lặp (mỗi vòng vài chu kỳ) cho ra khoảng 0.3–0.8 giây mỗi pha (tuỳ pipeline). Tốc độ khác nếu bật PLL/HSI/HSE.
	- Muốn nhanh/chậm hơn: chỉnh hằng số trong `delay()` hoặc cấu hình clock trước khi vào vòng for.

### Code flow (src/main.c)
1) Bật clock AHB2 cho GPIOC qua RCC_AHB2ENR.
2) Set PC6 thành output (GPIOC_MODER).
3) Vòng lặp vô hạn: set ODR bit 6 để bật LED, delay; clear bit 6 để tắt LED, delay.
4) Delay là busy-wait, không phụ thuộc SysTick hay timer.
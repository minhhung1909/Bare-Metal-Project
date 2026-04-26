# L2_SysTickTimer - Ghi chú học lại

Bài L2 thay cho delay vòng lặp của L1 bằng SysTick để có mốc thời gian ổn định hơn và ít phụ thuộc vào tối ưu hóa compiler.

## 1) Mục tiêu bài này

- Hiểu vì sao delay bằng vòng `for` không đáng tin khi đổi tần số hoặc mức tối ưu.
- Dùng SysTick của Cortex-M4 để tạo tick 1 ms.
- Viết `delay_ms()` dựa trên biến đếm tăng trong ISR.
- Nháy LED PC6 theo chu kỳ định trước bằng SysTick.

## 2) Vấn đề của delay kiểu bận (busy-wait)

Ở L1, delay phụ thuộc vào:

- Tần số CPU thực tế.
- Số chu kỳ cho mỗi vòng lặp.
- Mức tối ưu (`-O0`, `-O2`, `-O3`) của compiler.

Vì vậy cùng một hằng số delay có thể cho thời gian khác nhau trên mỗi cấu hình build.

## 3) SysTick trong bài này

SysTick là timer lõi (core peripheral) của ARM Cortex-M, có bộ đếm 24-bit đếm xuống.

Khi đếm từ giá trị reload về 0:

- Cờ COUNTFLAG được set.
- Nếu bật ngắt, CPU nhảy vào `SysTick_Handler()`.

## 4) Cấu hình theo code hiện tại

### 4.1 Các thanh ghi dùng

- `STK_LOAD` (0xE000E014): giá trị reload.
- `STK_VAL` (0xE000E018): giá trị đếm hiện tại.
- `STK_CTRL` (0xE000E010): enable, chọn clock và bật ngắt.

### 4.2 Tạo tick 1 ms

Code đang dùng `SYSCLK_HZ = 16000000`.

Giá trị reload:

$$reload = \frac{16{,}000{,}000}{1000} - 1 = 15999$$

Nếu `reload` vượt giới hạn 24-bit (`0x00FFFFFF`) thì hàm init trả lỗi.

### 4.3 Chuỗi khởi tạo trong `sysTick_Init()`

1. Kiểm tra `sysclk_hz >= 1000`.
2. Tính `reload = (sysclk_hz / 1000) - 1`.
3. Kiểm tra không vượt `STK_RELOAD_MAX`.
4. Ghi `STK_LOAD = reload`.
5. Ghi `STK_VAL = 0` để reset counter.
6. Bật `ENABLE`, `TICKINT`, `CLKSOURCE` trong `STK_CTRL`.

## 5) Luồng chương trình

Trong `main()`:

1. Gọi `sysTick_Init(SYSCLK_HZ)`.
2. Bật clock GPIOC và set PC6 output.
3. Vòng lặp vô hạn:
   - Bật LED.
   - `delay_ms(200)`.
   - Tắt LED.
   - `delay_ms(200)`.

Trong `SysTick_Handler()`:

- Tăng biến toàn cục `system_tick` mỗi 1 ms.

Trong `delay_ms(ms)`:

- Lưu mốc `startTime = system_tick`.
- Chờ đến khi `system_tick - startTime >= ms`.

## 6) Vector table và startup

SysTick exception nằm ở vị trí 15 trong bảng vector của Cortex-M.

Trong `startup_g4.c`, vector table đã trỏ đúng tới `SysTick_Handler`.

## 7) Build và flash

Trong thư mục `L2_SysTickTimer`:

```bash
make
make flash
```

Lưu ý trong Makefile đã bật `-g3 -O0` để thuận tiện debug.

## 8) Cách test nhanh

1. Flash firmware.
2. Quan sát LED PC6.
3. LED phải nháy với chu kỳ xấp xỉ 400 ms (200 ms bật + 200 ms tắt).

## 9) Lỗi hay gặp

- Quên map `SysTick_Handler` trong vector table.
- Không bật bit `TICKINT` nên `system_tick` không tăng.
- Không ghi `STK_VAL = 0` trước khi start.
- Nhầm tần số hệ thống, dẫn tới sai mốc ms.

## 10) Ghi nhớ nhanh

1. SysTick reload cho 1 ms: `sysclk/1000 - 1`.
2. ISR chỉ làm việc nhỏ: tăng `system_tick`.
3. `delay_ms()` dựa trên chênh lệch tick.
4. Muốn đúng thời gian thì phải chắc `SYSCLK_HZ` đúng.
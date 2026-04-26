# L4_AdvancedTimer_PWM - Ghi chú học lại

Bài L4 tập trung vào Advanced Timer TIM1 để tạo PWM bằng phần cứng trên chân PA8.

## 1) Mục tiêu bài này

- Hiểu cách tạo PWM bằng TIM1 mà không cần can thiệp CPU trong lúc chạy.
- Xuất PWM ra chân PA8 (TIM1_CH1, AF6).
- Thiết lập PWM tần số 50 Hz.
- Thiết lập duty ban đầu khoảng 10%.

## 2) Trạng thái code hiện tại

Trong `main()` đang gọi `init_PWM_PA8()` và chạy vòng lặp vô hạn.

Lưu ý:

- File vẫn còn hàm `init_TIM6()` và `TIM6_DAC_IRQHandler()` từ bài trước để tham khảo.
- Luồng PWM hiện tại không dùng ngắt TIM6.

## 3) Kiến thức cốt lõi về PWM

Ba thanh ghi quan trọng:

1. `PSC`: chia clock timer.
2. `ARR`: quyết định chu kỳ PWM.
3. `CCR1`: quyết định độ rộng xung mức cao (duty).

Công thức:

$$f_{PWM} = \frac{f_{TIM}}{(PSC + 1)(ARR + 1)}$$

$$Duty = \frac{CCR1}{ARR + 1} \times 100\%$$

## 4) Cấu hình theo code của bài này

### 4.1 Clock và chân output

- Bật clock GPIOA: `RCC_AHB2ENR` bit 0.
- Bật clock TIM1: `RCC_APB2ENR` bit 11.
- Set PA8 mode AF (`GPIOA_MODER` = `10`).
- Chọn AF6 cho PA8 (`GPIOA_AFRH`).

### 4.2 Tần số PWM

Code đang set:

- `TIM1_PSC = 160 - 1`
- `TIM1_ARR = 2000 - 1`

Với giả định clock TIM1 là 16 MHz:

$$f_{PWM} = \frac{16{,}000{,}000}{160 \times 2000} = 50\text{ Hz}$$

### 4.3 Duty ban đầu

Code set `TIM1_CCR1 = 200`.

$$Duty = \frac{200}{2000} \times 100\% = 10\%$$

### 4.4 Chế độ PWM và enable output

- `TIM1_CCMR1` set `OC1M = 110` (PWM Mode 1).
- Bật `OC1PE` (preload CCR1).
- `TIM1_CCER` bật `CC1E` để xuất ra kênh 1.
- `TIM1_BDTR` bật `MOE` (bit bắt buộc của Advanced Timer).
- `TIM1_CR1` bật `CEN` để timer bắt đầu đếm.

## 5) Build và flash

Trong thư mục `L4_AdvancedTimer_PWM`:

```bash
make
make flash
```

## 6) Cách test nhanh

1. Flash firmware.
2. Đo tại PA8 bằng oscilloscope/logic analyzer.
3. Kỳ vọng:
   - Tần số gần 50 Hz.
   - Duty khoảng 10%.

Nếu không có scope, có thể tạm nối LED + điện trở để quan sát mức sáng thay đổi khi thay `CCR1` (không chính xác bằng scope nhưng dễ kiểm tra nhanh).

## 7) Lỗi hay gặp

- Quên set AF6 cho PA8.
- Quên bật `MOE` trong `BDTR` (TIM1 không xuất xung dù timer vẫn chạy).
- Nhầm công thức `PSC/ARR` dẫn tới sai tần số.
- Viết nhầm `CCRx` ngoài khoảng `0..ARR+1`.

## 8) Hướng mở rộng

1. Viết hàm cập nhật duty runtime (ví dụ theo phần trăm).
2. Kết hợp ngắt/timer khác để sweep duty tự động.
3. Dùng nhiều channel TIM1 để xuất PWM đa kênh.
4. Tìm hiểu thêm dead-time/complementary output cho bài điều khiển công suất.

## 9) Ghi nhớ nhanh

1. PWM muốn ra được ở TIM1 phải có cả `CC1E` và `MOE`.
2. `PSC` + `ARR` quyết định tần số.
3. `CCR1` quyết định duty.
4. Cấu hình đúng AF6 trên PA8 trước khi bật timer.
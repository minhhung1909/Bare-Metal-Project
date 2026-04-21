Bài 4: Advaced Timer (TIM1)



![Mô tả PWM](docs/img/L4/STM32-PWM-Example-Tutorial-STM32-Timer-PWM-Mode.jpg)

So sánh thanh ghi counter và thanh ghi CCR thì sẽ xác định được xung PWM là 0 hay là 1. 


## 1. Mục tiêu

Project này hướng dẫn cách sử dụng bộ định thời nâng cao (Advanced-control Timer - TIM1) để tạo ra tín hiệu Điều chế độ rộng xung (PWM - Pulse Width Modulation). Tín hiệu PWM sinh ra được định tuyến vật lý ra chân PA8, phục vụ cho việc điều khiển tốc độ động cơ (ESC), điều khiển góc quay Servo hoặc thay đổi độ sáng LED.

Đặc điểm hệ thống:

Tần số xung đầu ra: 50Hz (Chuẩn điều khiển Servo).

Duty Cycle ban đầu: 10%.

Vi xử lý (CPU) được giải phóng 100% tài nguyên nhờ tính năng Hardware Autonomy (Phần cứng tự trị).
---

Dưới đây là bản tài liệu kỹ thuật được chuẩn hóa, mang đúng phong cách "Kỹ sư Hệ thống". Mày có thể copy toàn bộ nội dung này, lưu thành file `README_TIM1_PWM.md` ném lên Github hoặc kẹp vào hồ sơ thiết kế Flight Controller của mày.

---


## 2. Lõi Toán Học & Thanh Ghi

Toàn bộ hình thái của tín hiệu PWM được quyết định bởi 3 thanh ghi cốt lõi. Ghi nhớ quy tắc: *Phần cứng đếm từ 0, nên giá trị nạp vào Timer luôn là `N - 1`*.

1. **PSC (Prescaler):** Hộp số giảm tốc cho xung nhịp gốc.
2. **ARR (Auto-Reload Register):** Vạch trần nhà. Quyết định **Chu kỳ (Period)** và **Tần số (Frequency)**.
   * *Công thức tính Tần số PWM:*
     $$f_{PWM} = \frac{f_{TIM\_CLK}}{(PSC + 1) \times (ARR + 1)}$$
3. **CCRx (Capture/Compare Register):** Vạch đích phụ. Quyết định **Độ rộng xung (Duty Cycle)**.
   * *Công thức tính Duty Cycle:*
     $$Duty = \left( \frac{CCRx}{ARR + 1} \right) \times 100\%$$

---

## 3. Trình Tự Khởi Tạo Phần Cứng (PWM Mode 1)

Việc khởi động Advanced Timer yêu cầu tuân thủ nghiêm ngặt 7 bước sau:

1. **Cấp Nguồn (Clock):** Bật điện cho Bus GPIO tương ứng và Bus APB2 (nơi TIM1 trú ngụ).
2. **Trổ Cửa Sổ (Alternative Function - AF):** * Cấu hình chân GPIO ở chế độ `Alternate Function`.
   * Định tuyến chân đó vào đúng kênh của TIM1 (Ví dụ: `PA8` map với `AF6` để ra `TIM1_CH1`).
3. **Chốt Tần Số:** Nạp `PSC` và `ARR` theo chuẩn tần số mục tiêu (VD: 50Hz).
4. **Chốt Độ Rộng Xung:** Nạp giá trị khởi tạo vào `CCRx`.
5. **Cấu Hình Chế Độ (OCxM):** Ghi giá trị `0110` (PWM Mode 1) vào thanh ghi `CCMRx`. Khuyến nghị bật thêm bit Preload (`OCxPE`) để xung mượt hơn.
6. **Mở Cổng & Chốt An Toàn:** * Bật ngõ ra trên kênh vật lý (`CCxE` trong `CCER`).
   * **[TỬ HUYỆT]:** Bật Main Output Enable (`MOE` trong `BDTR`). Quên bit này, TIM1 sẽ bị khóa mõm hoàn toàn.
7. **Kích Nổ:** Bật Counter Enable (`CEN` trong `CR1`) để bắt đầu băm xung.
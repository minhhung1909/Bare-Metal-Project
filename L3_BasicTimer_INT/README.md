Bài 3: Basic Timer (TIM6) & Ngắt (Interrupt)

## 1. Mục tiêu
Project này hướng dẫn cách cấu hình bộ định thời cơ bản (**TIM6**) để tạo ra một chu kỳ ngắt chính xác **500ms** (0.5 giây), ứng dụng để chớp tắt LED trên chân **PC6**.

**Đặc điểm của project:**
* Không sử dụng hàm delay bằng vòng lặp (giúp giải phóng CPU cho các tác vụ khác).
* Không sử dụng thư viện HAL/LL.

---

## 2. Quy trình 5 bước cấu hình TIM6

Timer là một thiết bị ngoại vi độc lập. Để Timer hoạt động và sinh ra ngắt, cần thực hiện trình tự 5 bước sau trong hàm `init_TIM6()`:

1. **Cấp xung nhịp (Enable Clock):** Các ngoại vi mặc định không được cấp xung clock để tiết kiệm năng lượng. TIM6 thuộc bus APB1, do đó cần bật bit số 4 của thanh ghi `RCC->APB1ENR1`.
2. **Cấu hình bộ chia tần số (Prescaler - PSC) và Giá trị nạp lại (Auto-Reload - ARR):**
   * *Tần số hệ thống (System Clock):* 16.000.000 Hz.
   * *Thanh ghi PSC:* Chia tần số đầu vào. Cài đặt `TIM6->PSC = 1600 - 1` -> Tần số đếm của Timer giảm xuống còn 10.000 Hz (10.000 nhịp/giây).
   * *Thanh ghi ARR:* Đặt ngưỡng tràn bộ đếm. Cài đặt `TIM6->ARR = 5000 - 1` -> Đếm 5000 nhịp sẽ tương đương đúng 0.5 giây.
   * *Lưu ý phần cứng:* Các thanh ghi này luôn tự động cộng thêm 1 khi hoạt động, nên trong code bắt buộc phải trừ đi 1 (`Value - 1`).
3. **Kích hoạt ngắt nội bộ (Interrupt Enable):** Ghi giá trị 1 vào bit `UIE` (Update Interrupt Enable - Bit 0) của thanh ghi `DIER` để cho phép TIM6 phát tín hiệu ngắt khi đếm đến giá trị ARR.
4. **Cấp phép ngắt trên NVIC (Nested Vectored Interrupt Controller):** Mở cổng ngắt tương ứng của TIM6 trên bộ điều khiển ngắt của lõi ARM. Đối với STM32G431, ngắt TIM6 chia sẻ chung với DAC tại kênh số 54 (`TIM6_DAC_IRQn`).
5. **Kích hoạt Counter (Enable Timer):** Bật bit `CEN` (Counter Enable - Bit 0) trong thanh ghi `CR1` để Timer bắt đầu đếm.

---

## 3. Hàm xử lý ngắt (Interrupt Service Routine - ISR)

Khi TIM6 đếm xong chu kỳ 500ms, CPU sẽ tạm dừng chương trình chính và nhảy vào hàm xử lý ngắt.
**Lưu ý:** Tên hàm phải ghi chính xác tuyệt đối theo định nghĩa của tệp `startup`.

```c
void TIM6_DAC_IRQHandler(void){
    // 1. Kiểm tra cờ ngắt: Xác nhận ngắt được sinh ra do sự kiện Update của TIM6
    if (TIM6_SR & (1U << 0)) {
        // 2. QUAN TRỌNG: Xóa cờ ngắt (Clear Interrupt Flag)
        // Bắt buộc phải xóa cờ UIF bằng phần mềm. Nếu không xóa, 
        // CPU sẽ bị kẹt trong vòng lặp ngắt vô tận (Infinite Interrupt Loop).
        TIM6_SR &= ~(1U << 0);

        // 3. Thực thi tác vụ: Đảo trạng thái chân PC6 (Toggle LED) bằng phép toán XOR (^)
        GPIOC_ODR ^= (1U << 6);
    }
}
```

---

## 4. Bảng Vector Ngắt (Interrupt Vector Table)

Trong kiến trúc Bare-Metal tự viết tệp khởi động (`startup.c`), chúng ta phải đưa địa chỉ của hàm ngắt vào đúng vị trí trong Bảng Vector Ngắt để Trình liên kết (Linker) có thể tìm thấy.

* 16 vị trí đầu tiên dành cho Exception của lõi ARM (Reset, NMI, HardFault, SysTick...).
* Sự kiện `TIM6_DAC_IRQn` nằm ở vị trí số **54** trong bảng ngắt ngoại vi của ST.
=> Vị trí chính xác trong mảng là: `16 + 54`.

*Sử dụng cú pháp Designated Initializers của chuẩn C99:*
```c
__attribute__((section(".vectors"))) 
void (*const tab[16 + 91])(void) = {
    [0]  = (void (*)(void))&_estack,
    [1]  = _reset,
    [16 + 54] = TIM6_DAC_IRQHandler   // Đưa địa chỉ hàm ISR vào đúng slot của TIM6
};
```

---

## 5. Một số lưu ý khi biên dịch và nạp code

### Sử dụng ST-Link từ Windows qua máy ảo WSL2
Môi trường Linux (WSL2) mặc định không nhận thiết bị USB từ Windows. Cần sử dụng công cụ `usbipd` trên Windows PowerShell (Run as Administrator):
```powershell
usbipd list                                  # Liệt kê các thiết bị, tìm số BUSID của ST-Link
usbipd bind --busid <BUSID>                  # Đăng ký thiết bị với usbipd
usbipd attach --wsl --busid <BUSID>          # Chuyển hướng kết nối USB vào WSL
```
Sau đó, sử dụng lệnh `make flash` trên WSL để tiến hành nạp chương trình vào vi điều khiển.
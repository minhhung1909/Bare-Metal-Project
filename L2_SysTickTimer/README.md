# Vấn đề
Bài 2 này được tạo là để thay thế cái vòng for delay trong bài 1. Khi dùng hàm delay trong bài 1 (thường được gọi là software Timer) thì __asm("nop) được thực thi. Khi này ta không biết rỏ là bao nhiêu chu kì CPU ví dụ như đối với CPU chạy tần số là 16MHz là 1 giây đi thì với CPU 160MHz lúc này chỉ còn lại 0.1 giây thôi Thêm vào đó về vấn đề optimaze của compiler thì khi đổi từ `-O0` (không tối ưu) sang `-O3` (tối ưu tối đa), thằng GCC thấy cái vòng lặp vô nghĩa quá, nó xóa luôn hàm delay đi.

# Tổng quan về System Tick
System Tick là ngoại vi core của Cortex ARM M4. có 24 bit đếm ngược. Khi đếm về 0 thì kích hoạt ngắt. System Tick này thường được dùng để triển khai chức năng định thời cho các OS thường thấy nhất là trong RTOS khi cấu hình CubeMX khi bật FreeRTOS lên thì phải dùng timer khác vì systeam Tick bị dùng cho OS. 


Đầu tiên phải viết file startup `L2_SysTickTimer/Src/startup_g4.c`
File startup này khác startup kia ở chỗ cấu hình thêm `SysTick_Handler`. Cấu hình ở vị trí số 15 trong bảng vector table của chip Cortex arm M4 đọc tài liệu `PM0214`.

# Cách dùng System Tick
Vì là ngoại vi cơ bản của Chip M4 nên mặc định sẽ có sẵn và chỉ cần 4 thanh ghi để cấu hình. 

1. `SYST_CSR`:Điều khiển hoạt động của counter SysTick và cung cấp thông tin trạng thái

2. `SYST_RVR`: Reload Value Register Là thanh ghi chỉ định khởi đầu dùng để đặt giá trị khởi tạo của timer

Ví dụ: Muốn Clock Cycle là 1ms mà tần số chip là 16MHz thì phải là 16000 Cycle. Tức là đếm xuống 16000 - 1 là sẽ trigger. Lúc này sẽ set giá trị của register `SYST_RVR` là 15999. 

3. `SYST_CVR`: System Tick Current value register giữ giá trị hiện tại của counter. Có thể sử dụng register này để theo dõi counter. 

Note: Luôn ghi 0 vào đây ở bước cấu hình đầu tiên để dọn rác trước khi cho timer chạy.

4. `SYST_CALIB`: System Tick Calibration value: Register này cung cấp các properties hiệu chuẩn bộ đếm thời gian. 

Nói chung là mấy thanh ghi này trong tài liệu `PM0214` viết chi tiết nên có thể xem thêm.

# Makefile

Trong bài này có thay đổi 1 tí về makefile so với bài 1 là thêm header file

```Makefile
CFLAGS  ?=  -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
	-Wformat-truncation -fno-common -Wconversion \
	-g3 -O0 -ffunction-sections -fdata-sections -I. -I$(INC_DIR)\
	-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(EXTRA_CFLAGS)

```



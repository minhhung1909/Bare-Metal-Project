#include <stdint.h>

extern uint32_t _sbss, _ebss, _sdata, _edata, _sidata, _estack;
extern void SysTick_Handler(void);
extern int main(void);

// Hàm Reset Handler
__attribute__((naked, noreturn)) void _reset(void) {
    for (uint32_t *dst = &_sbss; dst < &_ebss; dst++) *dst = 0; //set lại 0 cho vùng .bss 
    for (uint32_t *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++; //copy dữ liệu từ flash đưa lên ram
    
    // Sau khi khởi tạo xong các vùng nhớ thì sẽ nhảy vào hàm main(). Trước đó là những gì mà trước khi main() được chạy.
    main();
    for (;;) (void)0;
    /*trong main(), CPU sẽ rơi vào vùng nhớ không xác định phía sau, 
    thực thi các mã rác và gây chập cháy hoặc hỏng hóc hệ thống. 
    Vòng lặp vô tận này là một cái bẫy chủ động để "giam" CPU lại 
    tại chỗ (treo an toàn) thay vì để nó chạy loạn. */
}

// Bảng Vector
__attribute__((section(".vectors"))) 
void (*const tab[16 + 91])(void) = {// 16 là System Exceptions của arm còn 19 là External Interrupts của ST
    (void (*)(void))&_estack,       // <-- 4 byte đầu tiên
    _reset,                          // <-- 4 byte tiếp theo
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0,
    0,
    SysTick_Handler
};
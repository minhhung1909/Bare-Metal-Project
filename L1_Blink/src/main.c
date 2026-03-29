#include <stdint.h>
#include <stdbool.h>

#define RCC_BASE            0x40021000
#define RCC_AHB2ENR         (*(volatile unsigned int *)(RCC_BASE + 0x4C))
#define PORTC_BASE          0x48000800
#define GPIOC_MODER         (*(volatile unsigned int *)(PORTC_BASE + 0x00))
#define GPIOC_ODR           (*(volatile unsigned int *)(PORTC_BASE + 0x14))

void delay(volatile uint32_t count) {
    while(count--) {
        __asm("nop"); // Thêm lệnh này để ép CPU thực hiện 1 chu kỳ trống
    }
}

int main(void) {

    RCC_AHB2ENR |= (1U << 2); // Power supply to port C.

    GPIOC_MODER &= ~( 3U << (6*2) );
    GPIOC_MODER |= ( 1U << (6*2) );

    for (;;) {
        GPIOC_ODR |= (1U << 6); 
        delay(5000000); 

        GPIOC_ODR &= ~(1U << 6); 
        delay(5000000);
    }
    return 0;
}
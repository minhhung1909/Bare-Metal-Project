#include <stdint.h>
#include <stdbool.h>
#include "Systick.h"

#define RCC_BASE            0x40021000
#define RCC_AHB2ENR         (*(volatile unsigned int *)(RCC_BASE + 0x4C))
#define PORTC_BASE          0x48000800
#define GPIOC_MODER         (*(volatile unsigned int *)(PORTC_BASE + 0x00))
#define GPIOC_ODR           (*(volatile unsigned int *)(PORTC_BASE + 0x14))
#define SYSCLK_HZ           (16000000U)

static inline void __enable_irq(void) {
    __asm volatile ("cpsie i" : : : "memory");
}


int main(void) {
    *(volatile uint32_t*)(0xE000ED08) = 0x08012000;
    __enable_irq();

    if (sysTick_Init(SYSCLK_HZ) != 0) {
        for (;;) {}
    }

    RCC_AHB2ENR |= (1U << 2); // Power supply to port C.

    GPIOC_MODER &= ~( 3U << (6*2) );
    GPIOC_MODER |= ( 1U << (6*2) );

    for (;;) {
        GPIOC_ODR |= (1U << 6); 
        delay_ms(5000); 

        GPIOC_ODR &= ~(1U << 6); 
        delay_ms(5000);
    }
    return 0;
}
#include "Systick.h"
#include <stdint.h>
#include <stdbool.h>

#define RCC_BASE_ADRR           0x40021000
#define PORTC_BASE              0x48000800
#define TIM6_BASE_ADRR          0x40001000

#define RCC_AHB2ENR             (*(volatile unsigned int *)(RCC_BASE_ADRR + 0x4C))
#define GPIOC_MODER             (*(volatile unsigned int *)(PORTC_BASE + 0x00))
#define GPIOC_ODR               (*(volatile unsigned int *)(PORTC_BASE + 0x14))
#define RCC_APB1ENR1            (*(volatile unsigned int *)(RCC_BASE_ADRR + 0x58))

#define TIM6_PSC_ADRR           (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x28))
#define TIM6_ARR_ADRR           (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x2C))
#define TIM6_DMA_INT_ENB        (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x0C))
#define TIM6_CONTROL_1          (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x00))
#define TIM6_SR                 (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x10))

#define PRESCALER                1600-1
#define AUTORELOAD               5000-1

#define NVIC_ISER1              (*(volatile unsigned int *) (0xE000E104))

#define SYSCLK_HZ               (16000000U)


void init_TIM6(void){
    RCC_APB1ENR1 |= (1U << 4);
    TIM6_PSC_ADRR = PRESCALER;
    TIM6_ARR_ADRR = AUTORELOAD;
    TIM6_DMA_INT_ENB |= (1U << 0);

    NVIC_ISER1 |= (1U << 22);   //Ngắt 54 nằm ở thanh ghi ISER1 (vị trí bit: 54 - 32 = 22)

    TIM6_CONTROL_1 |= (1u << 0);

}

void init_led(void){
    RCC_AHB2ENR |= (1U << 2);

    GPIOC_MODER &= ~( 3U << (6*2) );
    GPIOC_MODER |= ( 1U << (6*2) );
}

void TIM6_DAC_IRQHandler(void){
    if (TIM6_SR & (1U << 0)) {

        TIM6_SR &= ~(1U << 0);

        GPIOC_ODR ^= (1U << 6);
    }
}

int main(void) {
    init_TIM6();
    init_led();
    for (;;) {

    }
    return 0;
}
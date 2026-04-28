#include <stdint.h>
#include <stdbool.h>
#include "register.h"
#include "Systick.h"

uint8_t firmware_receiver[256];

void init_led(){
    RCC_AHB2ENR |= (1U << 2); // Power supply to port C.

    GPIOC_MODER &= ~( 3U << (6*2) );
    GPIOC_MODER |= ( 1U << (6*2) );
}

void toggle_led()
{
    GPIOC_ODR ^= (1U << 6);
}

static inline void __enable_irq(void) {
    __asm volatile ("cpsie i" : : : "memory");
}

void DMA1_Channel1_IRQHandler(void)
{
    /*  
    *   Bit 1 CTCIF1: transfer complete flag clear for channel 1
    *   Bit 0 CGIF1: global interrupt flag clear for channel 1 
    */
    DMA_IFCR |= (1U << 1) | (1U << 0);
}

int main(void) {
    // relocation vector table
    *(volatile uint32_t*)(0xE000ED08) = 0x08004000;
    
    __enable_irq();
    
    if (sysTick_Init(SYSCLK_HZ) != 0) {
        for (;;) {}
    }

    init_led();
    uart_init();
    DMA_init(firmware_receiver, sizeof(firmware_receiver));

    for (;;) {
        toggle_led();
        delay_ms(200);
    }
    return 0;
}
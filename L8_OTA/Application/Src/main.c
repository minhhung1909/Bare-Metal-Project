#include <stdint.h>
#include <stdbool.h>
#include "register.h"
#include "Systick.h"
#include "uart.h"
#include "flash.h"
#include "crc.h"
#include "ota.h"
#include "dma.h"

__attribute__((aligned(8))) uint8_t firmware_buffer[MAX_SIZE_BUFF];

volatile bool flag_ota = false;

static inline void __enable_irq(void)
{
    __asm volatile("cpsie i" : : : "memory");
}

void init_led()
{
    RCC_AHB2ENR |= (1U << 2);
    GPIOC_MODER &= ~(3U << (6 * 2));
    GPIOC_MODER |= (1U << (6 * 2));
}

void toggle_led()
{
    GPIOC_ODR ^= (1U << 6);
}

void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_ISR & (1U << 1)) 
    {
        /* * Xóa cờ ngắt. Trên STM32, thanh ghi ISR là dạng Chỉ đọc (Read-Only). 
         * Xóa cờ TCIF1 (Bit 1) và CGIF1 (Global Interrupt - Bit 0)
         */
        DMA1_IFCR |= (1U << 1) | (1U << 0);
        
        flag_ota = true;
    }
}

int main(void)
{
    __enable_irq();

    // Set vector table to current app slot
    *(volatile uint32_t *)(0xE000ED08) = 0x08004000;

    if (sysTick_Init(SYSCLK_HZ) != 0)
    {
        for (;;)
        {
        }
    }

    init_led();
    uart_init();
    Hardware_CRC_Init();
    DMA_init(firmware_buffer, MAX_SIZE_BUFF);

    for (;;)
    {

        if (flag_ota == true)
        {
            OTA_Process_Buffer(firmware_buffer);

            flag_ota = false;
            
            DMA_CCR1 &= ~(1U << 0);
    
            DMA_CNDTR1 = MAX_SIZE_BUFF;      
    
            DMA_CCR1 |= (1U << 0);
        }
        toggle_led();
        delay_ms(500);
    }

    return 0;
}
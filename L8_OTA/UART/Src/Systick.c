#include "Systick.h"

#define STK_CTRL            (*(volatile uint32_t *)(0xE000E010U))
#define STK_LOAD            (*(volatile uint32_t *)(0xE000E014U))
#define STK_VAL             (*(volatile uint32_t *)(0xE000E018U))
#define STK_RELOAD_MAX      (0x00FFFFFFU)

#define STK_CTRL_ENABLE     (1U << 0)
#define STK_CTRL_TICKINT    (1U << 1)
#define STK_CTRL_CLKSOURCE  (1U << 2)

volatile uint32_t system_tick = 0;


void SysTick_Handler(void) {
    system_tick++;
}

int sysTick_Init(uint32_t sysclk_hz)
{
    if (sysclk_hz < 1000U) {
        return -1;
    }

    uint32_t reload = (sysclk_hz / 1000U) - 1U;
    if (reload > STK_RELOAD_MAX) {
        return -1;
    }

    STK_LOAD = reload;

    STK_VAL = 0;
    
    STK_CTRL = STK_CTRL_ENABLE | STK_CTRL_TICKINT | STK_CTRL_CLKSOURCE;
    return 0;
}

void delay_ms(uint32_t ms)
{
    uint32_t startTime = system_tick;

    while((system_tick - startTime) < ms){}
}
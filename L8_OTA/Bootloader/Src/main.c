#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "Systick.h"
#include "register.h"

// hàm tắt ngắt toàn cục bằng lệnh Assembly "cpsid i" của lõi Cortex-M
static inline void __disable_irq(void)
{
    __asm volatile("cpsid i" : : : "memory");
}

// Hàm Set giá trị cho thanh ghi Stack Pointer (MSP) bằng lệnh Assembly "msr"
static inline void __set_MSP(uint32_t topOfMainStack)
{
    __asm volatile("MSR msp, %0" : : "r"(topOfMainStack) :);
}

#define ADDRESS_FLAG_APP_CHECK 0x0801E800
#define ADDRESS_FLAG_APP1 0x08004000U
#define ADDRESS_FLAG_APP2 0x08012000U

typedef void (*pFunction)(void);
void Jump_To_App(uint32_t app_address)
{
    // 1. Disable interrup
    __disable_irq();

    // Bypass 4byte to resethandler
    uint32_t jump_address = *(volatile uint32_t *)(app_address + 4);

    pFunction Jump_To_Application = (pFunction)jump_address;

    // 4. Set MSP for new app (because before bypass 4 byte)
    __set_MSP(*(volatile uint32_t *)app_address);

    Jump_To_Application();
}

int main(void)
{
    RCC_AHB2ENR |= (1U << 2);

    GPIOC_MODER &= ~(3U << 26);

    GPIOC_PUPDR &= ~(3U << 26); // Xóa sạch rác
    GPIOC_PUPDR |= (2U << 26);  // Ghi số 2 (10 hệ nhị phân) vào vị trí 26

    for (volatile int i = 0; i < 100; i++)
        ;

    if ((GPIOC_IDR & (1U << 13)) != 0)
    {
        Jump_To_App(ADDRESS_FLAG_APP2);
    }
    else
    {
        Jump_To_App(ADDRESS_FLAG_APP1);
    }

    while (1)
        ;
}
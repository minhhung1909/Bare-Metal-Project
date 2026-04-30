#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "Systick.h"
#include "register.h"
#include "config.h"
#include "uart.h"

#define FLAG_ADDR 0x0801E800 // page 61

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
    uart_init();
    OTA_Shared_Config_t *ota_shared = (volatile OTA_Shared_Config_t*) FLAG_ADDR;
    char text[] = "hello from bootloader \r\n";
    UART_Transmit_multi(text, sizeof(text));
    // Not first times
    char app1[] = "hello jump to app 1 \r\n";
    char app2[] = "hello jump to app 2 \r\n";
    
    if (ota_shared->Magic_Word == 0xDEADBEEF)
    {
        if(ota_shared->Active_App == 1){
            UART_Transmit_multi(app1, sizeof(app1));
            Jump_To_App(ADDRESS_FLAG_APP1);
        }
        else{
            UART_Transmit_multi(app2, sizeof(app2));
            Jump_To_App(ADDRESS_FLAG_APP2);
        }
    }else{
        Jump_To_App(ADDRESS_FLAG_APP1);
    }

    while (1);
}
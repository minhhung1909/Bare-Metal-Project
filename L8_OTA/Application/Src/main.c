#include <stdint.h>
#include <stdbool.h>
#include "register.h"
#include "Systick.h"
#include "uart.h"
#include "flash.h"
#include "crc.h"
#include "ota.h"

__attribute__((aligned(8))) uint8_t firmware_buffer[MAX_FIRMWARE_SIZE];

void init_led(){
    RCC_AHB2ENR |= (1U << 2);
    GPIOC_MODER &= ~(3U << (6*2));
    GPIOC_MODER |= (1U << (6*2));
}

void toggle_led()
{
    GPIOC_ODR ^= (1U << 6);
}

void DMA1_Channel1_IRQHandler(void)
{
}

int main(void) {
    // Set vector table to current app slot
    *(volatile uint32_t*)(0xE000ED08) = 0x08004000;
    
    if (sysTick_Init(SYSCLK_HZ) != 0) {
        for (;;) {}
    }

    init_led();
    uart_init();
    Hardware_CRC_Init();
    
    const char ready[] = "\r\n=== OTA Ready ===\r\nSend packet: [Magic][Len][Reserved][CRC][Firmware]\r\n";
    UART_Transmit_multi((char*)ready, sizeof(ready) - 1);

    for (;;) {
        const char prompt[] = "Waiting for OTA packet...\r\n";
        UART_Transmit_multi((char*)prompt, sizeof(prompt) - 1);
        
        uint16_t firmware_len;
        uint32_t firmware_crc;
        
        // Receive OTA packet
        uint8_t result = receive_ota_packet(firmware_buffer, &firmware_len, &firmware_crc);
        
        if (result == 0) {
            // CRC validated, program to opposite slot
            program_ota_firmware(firmware_buffer, firmware_len, firmware_crc);
        }
        
        toggle_led();
        delay_ms(500);
    }
    
    return 0;
}
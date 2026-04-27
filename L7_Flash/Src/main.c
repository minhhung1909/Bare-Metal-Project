#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "Systick.h"
#include "register.h"
#include "uart.h"
#include "flash.h"

// char data_recei[20];
// int rx_index = 0;
uint8_t flash_buffer[30] __attribute__((aligned(8)));

void init_led(void)
{
    RCC_AHB2ENR |= (1U << 2);

    GPIOC_MODER &= ~( 3U << (6*2) );
    GPIOC_MODER |= ( 1U << (6*2) );
}

void toggo_led()
{
    GPIOC_ODR ^= (1U << 6);
}

int main(void) 
{
    char arr[] = "Hello World Le Minh Hung \r\n";
    // padding dữ liệu
    memset(flash_buffer, 0xFF, sizeof(flash_buffer) );
    memcpy(flash_buffer, arr, sizeof(arr));
    if (sysTick_Init(SYSCLK_HZ) != 0) {
        for (;;) {}
    }

    delay_ms(1000);
    
    uart_init();
    init_led();

    
    ErasePage(48);
    uint16_t double_words_count = (sizeof(flash_buffer) + 7) / 8;
    // Program page 48
    Program_Flash((void *) 0x08018000, (uint64_t*)flash_buffer, double_words_count);

    for (;;) {
        toggo_led();
        delay_ms(500);
    }
    return 0;
}
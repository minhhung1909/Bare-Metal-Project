#include "Systick.h"
#include <stdint.h>
#include <stdbool.h>
#include "register.h"
#include "uart.h"
#include "dma.h"

char data_recei[20];
int rx_index = 0;

void init_led(void){
    RCC_AHB2ENR |= (1U << 2);

    GPIOC_MODER &= ~( 3U << (6*2) );
    GPIOC_MODER |= ( 1U << (6*2) );
}

void toggo_led()
{
    GPIOC_ODR ^= (1U << 6);
}

void DMA_Channel_1_IQRHandler(void)
{
    DMA1_IFCR |= (1U << 1) | (1U << 0);
    __asm("NOP");
}

int main(void) {
    char arr[] = "Hello World \r\n";
    if (sysTick_Init(SYSCLK_HZ) != 0) {
        for (;;) {}
    }

    uart_init();
    DMA_init(data_recei, sizeof(data_recei));
    init_led();
    UART_Transmit('H');
    UART_Transmit_multi(arr, sizeof(arr));
    for (;;) {
        toggo_led();
        delay_ms(500);
    }
    return 0;
}
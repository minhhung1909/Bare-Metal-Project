#include "Systick.h"
#include <stdint.h>
#include <stdbool.h>
#include "register.h"
#include "uart.h"

char data_recei[128];
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

void UART2_Interrup_Handler(void){
    
    data_recei[rx_index] = (char)USART2_RDR;
    rx_index++;
}

int main(void) {
    char arr[] = "Hello World \r\n";
    if (sysTick_Init(SYSCLK_HZ) != 0) {
        for (;;) {}
    }

    // uart_init(); // Khi không dùng ngắt
    UART_INIT_IVIC(); // Khi dùng ngắt
    init_led();
    UART_Transmit('H');
    UART_Transmit_multi(arr, sizeof(arr));
    delay_ms(100);
    for (;;) {
        /* recei bình thường không ngắt 
        data_recei[rx_index] = UART_Receiver();
        rx_index++;
        */
        toggo_led();
        delay_ms(500);
    }
    return 0;
}
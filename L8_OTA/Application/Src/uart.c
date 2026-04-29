#include "uart.h"
#include <stdint.h>
#include <stdbool.h>

#define APB1_CLOCK 16000000U
#define BAUDRATE   9600U

void uart_init()
{
    /* Uart 2 PA2 (TX) PA3 (RX)*/
    RCC_AHB2ENR |= (0b1 << 0);      //  Clock for port
    
    GPIOA_MODER &= ~(0b11U << 4 | 0b11U << 6);
    GPIOA_MODER |= ((0b10U << 4) | (0b10U << 6));
    
    GPIOA_AFRL &= ~(0b1111U << 8 | 0b1111U << 12);
    GPIOA_AFRL |= ((7 << 8) | (7 << 12));
 
    RCC_APB1ENR1 |= (0b1U << 17);    //  Clock for uart2

    /* Baud is 9600 */

    USART2_BRR = APB1_CLOCK / BAUDRATE;

    /* default: 8 bit data and no use parity
    so no need set bit data and parity
    */

    
    USART2_CR1 |= (0b1U << 0 | 0b1U << 2 | 0b1U << 3) ;

    USART2_ICR |= (1U << 3);
}


void UART_INIT_IVIC()
{
    /* Uart 2 PA2 (TX) PA3 (RX)*/
    RCC_AHB2ENR |= (0b1 << 0);      //  Clock for port
    
    GPIOA_MODER &= ~(0b11U << 4 | 0b11U << 6);
    GPIOA_MODER |= ((0b10U << 4) | (0b10U << 6));
    
    GPIOA_AFRL &= ~(0b1111U << 8 | 0b1111U << 12);
    GPIOA_AFRL |= ((7 << 8) | (7 << 12));
 
    RCC_APB1ENR1 |= (0b1U << 17);    //  Clock for uart2

    /* Baud is 9600 */

    USART2_BRR = APB1_CLOCK / BAUDRATE;

    /* default: 8 bit data and no use parity
    so no need set bit data and parity
    */

    
    USART2_CR1 |= (0b1U << 0 | 0b1U << 2 | 0b1U << 3 | 0b1U << 5) ;

    // ISER1 (38-32) bit 6
    // Vì ISER1 cấu hình nvic từ vị trí số 32 -> 63 mà ngắt usart2 nằm ở 38 nên 38 - 32 là bit số 6 nên bật bit số 6 
    NVIC_ISER1 |= (1U << 6);
}


void UART_INIT_DMA()
{
    /* Uart 2 PA2 (TX) PA3 (RX)*/
    RCC_AHB2ENR |= (0b1 << 0);      //  Clock for port
    
    GPIOA_MODER &= ~(0b11U << 4 | 0b11U << 6);
    GPIOA_MODER |= ((0b10U << 4) | (0b10U << 6));
    
    GPIOA_AFRL &= ~(0b1111U << 8 | 0b1111U << 12);
    GPIOA_AFRL |= ((7 << 8) | (7 << 12));
 
    RCC_APB1ENR1 |= (0b1U << 17);    //  Clock for uart2

    /* Baud is 9600 */

    USART2_BRR = APB1_CLOCK / BAUDRATE;

    /* default: 8 bit data and no use parity
    so no need set bit data and parity
    */

    
    USART2_CR1 |= (0b1U << 0 | 0b1U << 2 | 0b1U << 3) ;

    //Enable DMA for RX
    USART2_CR3 |= (1U << 6);
}

void UART_Transmit(char data) 
{
    while ((USART2_ISR & (1U << 7)) == 0);  //Waiting for data empty
    USART2_TDR = data;
    while ((USART2_ISR & (1U << 6)) == 0);  //Waiting for data transmit complete
}

void UART_Transmit_multi(char* arr, uint8_t size) 
{
    for (int i = 0; i < size; i++){
        while ((USART2_ISR & (1U << 7)) == 0);  //Waiting for data empty
        USART2_TDR = arr[i];
    }
    while ((USART2_ISR & (1U << 6)) == 0);  //Waiting for data transmit complete
}

char UART_Receiver() {
    // USART2_ISR
    while( (USART2_ISR & (1U << 5)) == 0);
    char data = (char )USART2_RDR;
    return data;
}
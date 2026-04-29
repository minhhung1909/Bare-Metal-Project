#ifndef UART_H
#define UART_H
#include <stdint.h>
#include <stdbool.h>
#include "register.h"

void uart_init();
void UART_INIT_IVIC();
void UART_Transmit(char data);
void UART_Transmit_multi(char* arr, uint8_t size);
char UART_Receiver();

#endif
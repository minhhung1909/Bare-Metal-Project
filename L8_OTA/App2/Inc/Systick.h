#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

int sysTick_Init(uint32_t sysclk_hz);
void delay_ms(uint32_t ms);

#endif
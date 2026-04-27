#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include <stdbool.h>

bool ErasePage(uint8_t page);
void Program_Flash(void* adrr, uint64_t *data_array, uint16_t size);

#endif
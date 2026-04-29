#include <stdint.h>

void Hardware_CRC_Init(void);
uint32_t Hardware_CRC_Calculate(uint8_t *data, uint16_t length_in_bytes);
uint32_t Calculate_Flash_CRC(uint32_t start_address, uint32_t total_bytes);
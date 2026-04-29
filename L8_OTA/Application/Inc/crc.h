#include <stdint.h>
void Hardware_CRC_Init(void);
uint32_t Hardware_CRC_Calculate(uint8_t *data, uint16_t length_in_bytes);

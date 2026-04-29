#include "crc.h"
#include "register.h"

void Hardware_CRC_Init(void) 
{
    RCC_AHB1ENR |= (1U << 12);
}

uint32_t Hardware_CRC_Calculate(uint8_t *data, uint16_t length_in_bytes)
{
    /* Bit 0 reset crc*/
    CRC_CR |= (1U << 0);

    uint32_t *pData32 = (uint32_t *)data;
    
    /* bytes data / 4 */
    uint16_t words_count = length_in_bytes / 4;
    uint8_t remainder = (uint8_t)(length_in_bytes % 4U);
    
    /* Cal crc */
    for (uint16_t i = 0; i < words_count; i++) {
        CRC_DR = pData32[i]; 
    }

    if (remainder != 0U) {
        uint32_t last_word = 0U;
        uint8_t *pLast = (uint8_t *)&last_word;
        uint16_t offset = (uint16_t)(words_count * 4U);

        for (uint8_t i = 0; i < remainder; i++) {
            pLast[i] = data[offset + i];
        }

        CRC_DR = last_word;
    }
    
    return CRC_DR;
}

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
    for (uint16_t i = 0; i < words_count; i++)
    {
        CRC_DR = pData32[i];
    }

    if (remainder != 0U)
    {
        uint32_t last_word = 0U;
        uint8_t *pLast = (uint8_t *)&last_word;
        uint16_t offset = (uint16_t)(words_count * 4U);

        for (uint8_t i = 0; i < remainder; i++)
        {
            pLast[i] = data[offset + i];
        }

        CRC_DR = last_word;
    }

    return CRC_DR;
}

uint32_t Calculate_Flash_CRC(uint32_t start_address, uint32_t total_bytes)
{
    /* Bit 0 reset crc*/
    CRC_CR |= (1U << 0);

    uint32_t *flash_ptr = (uint32_t *)start_address;

    uint32_t words_count = total_bytes / 4;
    uint8_t remainder = (uint8_t)(total_bytes % 4U);

    for (uint32_t i = 0; i < words_count; i++)
    {
        CRC_DR = flash_ptr[i];
    }

    if (remainder != 0U)
    {
        uint32_t last_word = 0U;
        uint8_t *pLast = (uint8_t *)&last_word;
        uint8_t *pFlash8 = (uint8_t *)start_address;
        uint32_t offset = words_count * 4U;

        for (uint8_t i = 0; i < remainder; i++)
        {
            pLast[i] = pFlash8[offset + i];
        }

        CRC_DR = last_word;
    }

    return CRC_DR;
}
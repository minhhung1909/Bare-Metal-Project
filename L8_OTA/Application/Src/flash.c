#include "flash.h"
#include "register.h"


bool ErasePage(uint8_t page)
{
    if (page > 63) return false;
    while (FLASH_SR & (1U << 16)); // Waiting for out of busy
    // Unlock chip if Locked
    if(FLASH_CR & (1U << 31))
    {
        FLASH_KEYR = 0x45670123;
        FLASH_KEYR = 0xCDEF89AB;
    }

    // Check bit error from 3 -> 8
    if (FLASH_SR & 0x3F8U ){
        FLASH_SR |= 0x3F8U; //  Clear bit if error
    }
    
    FLASH_CR &= ~(0x3FU << 3);
    // bit 3->8 is chosse page to erase.
    // bit 1 is PER page erase enable
    FLASH_CR |= (page << 3 | 1U << 1);
    //Trigger Erase
    FLASH_CR |= (1U << 16);

    while (FLASH_SR & (1U << 16)); // Waiting for out of busy

    // Deactivate PER.
    FLASH_CR &= ~(1U << 1);
    return true;
}

void Program_Flash(void* adrr, uint64_t *data_array, uint16_t size)
{
    while (FLASH_SR & (1U << 16)); // Waiting for out of busy
    // Check bit error from 3 -> 8
    if (FLASH_SR & 0x3F8U ){
        FLASH_SR |= 0x3F8U; //  Clear bit if error
    }

    // BIT 0: Flash programming enabled
    FLASH_CR |= (1U << 0);

    volatile uint32_t *write_addr = (uint32_t *)adrr;
    for (int i = 0; i < size; i++)
    {
        uint32_t data_low = (uint32_t) data_array[i];
        uint32_t data_high = (uint32_t) (data_array[i] >> 32);

        *write_addr = data_low;
        write_addr += 1;
        *write_addr = data_high;
    
        while (FLASH_SR & (1U << 16)); // Waiting for out of busy
        // CLear EOP after write success
        if (FLASH_SR & (1U << 0)) {
            FLASH_SR |= (1U << 0);
        }

        // jump to next addr
        write_addr += 1;
    }    
    // turn off PG
    FLASH_CR &= ~(1U << 0);
}
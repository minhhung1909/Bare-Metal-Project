#include <string.h>
#include <stdbool.h>
#include "ota.h"
#include "uart.h"
#include "crc.h"
#include "flash.h"
#include "register.h"

// Receive OTA packet header + firmware via polling UART
// Packet format: [4B magic][2B len][2B reserved][4B CRC][N bytes firmware]
uint8_t receive_ota_packet(uint8_t *firmware_buf, uint16_t *out_len, uint32_t *out_crc)
{
    OTA_Header_t header;
    uint8_t *p = (uint8_t *)&header;
    
    // Receive 12-byte header
    for (uint16_t i = 0; i < (uint16_t)sizeof(OTA_Header_t); i++) {
        p[i] = UART_Receiver();
    }
    
    // Validate magic
    if (header.magic != OTA_MAGIC) {
        const char err[] = "ERR: Invalid magic\r\n";
        UART_Transmit_multi((char*)err, sizeof(err) - 1);
        return 1;
    }
    
    // Validate firmware length
    if (header.firmware_len == 0 || header.firmware_len > MAX_FIRMWARE_SIZE) {
        const char err[] = "ERR: Invalid firmware length\r\n";
        UART_Transmit_multi((char*)err, sizeof(err) - 1);
        return 2;
    }
    
    // Receive firmware data
    for (uint16_t i = 0; i < header.firmware_len; i++) {
        firmware_buf[i] = UART_Receiver();
    }
    
    // Calculate CRC on received firmware
    uint32_t calc_crc = Hardware_CRC_Calculate(firmware_buf, header.firmware_len);
    
    // Validate CRC
    if (calc_crc != header.firmware_crc) {
        const char err[] = "ERR: CRC mismatch\r\n";
        UART_Transmit_multi((char*)err, sizeof(err) - 1);
        return 3;
    }
    
    *out_len = header.firmware_len;
    *out_crc = header.firmware_crc;
    
    const char ok[] = "OK: Packet received\r\n";
    UART_Transmit_multi((char*)ok, sizeof(ok) - 1);
    
    return 0;  // Success
}

// Program firmware to opposite slot
void program_ota_firmware(uint8_t *firmware_buf, uint16_t firmware_len, uint32_t firmware_crc)
{
    (void)firmware_crc;  // Parameter used for validation in receive, not needed here
    
    // Detect current app location from VTOR
    uint32_t current_vtor = *(volatile uint32_t*)(0xE000ED08);
    uint32_t target_addr;
    uint8_t target_page_start;
    uint8_t next_app_slot;

    if (current_vtor == 0x08004000) {
        // Running App slot 1 → program slot 2
        target_addr = 0x08012000;
        target_page_start = 18;  // Page 18-23 (6 pages)
        next_app_slot = 2;
        const char msg[] = "Target: App2 slot\r\n";
        UART_Transmit_multi((char*)msg, sizeof(msg) - 1);
    } else {
        // Running App slot 2 → program slot 1
        target_addr = 0x08004000;
        target_page_start = 8;   // Page 8-13 (6 pages)
        next_app_slot = 1;
        const char msg[] = "Target: App1 slot\r\n";
        UART_Transmit_multi((char*)msg, sizeof(msg) - 1);
    }

    // Erase target pages (6 pages × 2KB = 12KB)
    for (uint8_t page = target_page_start; page < (uint8_t)(target_page_start + 6); page++) {
        ErasePage(page);
    }

    // Program firmware into target slot
    uint16_t double_words_count = (uint16_t)((firmware_len + 7U) / 8U);
    Program_Flash((void*)target_addr, (uint64_t*)firmware_buf, double_words_count);

    // Update bootloader metadata (set active_app flag at 0x0801E800 + offset 1)
    uint32_t *pOTA = (uint32_t*)0x0801E800;
    *(pOTA + 1) = next_app_slot;  // Active_App field

    const char done[] = "Done. Rebooting...\r\n";
    UART_Transmit_multi((char*)done, sizeof(done) - 1);

    // Wait for UART to finish
    for (volatile uint32_t i = 0; i < 500000UL; i++);

    // System reset via SCB AIRCR
    *(volatile uint32_t*)0xE000ED0C = (0x5FAU << 16) | (1U << 2);
}

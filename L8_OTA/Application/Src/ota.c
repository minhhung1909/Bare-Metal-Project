#include <string.h>
#include <stdbool.h>
#include "ota.h"
#include "uart.h"
#include "crc.h"
#include "flash.h"
#include "register.h"
#include "config.h"

static uint16_t expected_seq = 0;
static uint32_t current_flash_addr = 0;

static uint32_t incoming_version = 0;
static uint32_t incoming_total_bytes = 0;
static uint16_t incoming_total_frames = 0;
static uint32_t incoming_global_crc = 0;
typedef void (*pFunction)(void);

// Hàm Set giá trị cho thanh ghi Stack Pointer (MSP) bằng lệnh Assembly "msr"
static inline void __set_MSP(uint32_t topOfMainStack)
{
    __asm volatile("MSR msp, %0" : : "r"(topOfMainStack) :);
}

static inline void __disable_irq(void)
{
    __asm volatile("cpsid i" : : : "memory");
}

static inline void __enable_irq(void)
{
    __asm volatile("cpsie i" : : : "memory");
}

static uint32_t get_location_app()
{
    // Detect current app location from VTOR
    uint32_t current_vtor = *(volatile uint32_t *)(0xE000ED08);
    return (current_vtor == ADDRESS_FLAG_APP1) ? ADDRESS_FLAG_APP2 : ADDRESS_FLAG_APP1;
}


static void Write_OtaShared(void)
{
    OTA_Shared_Config_t ota_shared;
    
    memset(&ota_shared, 0xFF, sizeof(OTA_Shared_Config_t));

    ota_shared.Magic_Word = 0xDEADBEEF;
    
    uint32_t target_app = get_location_app();
    ota_shared.Active_App = (target_app == ADDRESS_FLAG_APP1) ? 1 : 2;

    __disable_irq();
    ErasePage(61);
    
    uint16_t dwords = (sizeof(OTA_Shared_Config_t) + 7) / 8;
    Program_Flash((void *)0x0801E800U, (uint64_t *)&ota_shared, dwords);
    
    __enable_irq();
}

void Jump_To_App(uint32_t app_address)
{
    // Bypass 4byte to resethandler
    uint32_t jump_address = *(volatile uint32_t *)(app_address + 4);

    pFunction Jump_To_Application = (pFunction)jump_address;

    // Set MSP for new app (because before bypass 4 byte)
    __set_MSP(*(volatile uint32_t *)app_address);

    Jump_To_Application();
}

static void determent_erase()
{
    uint32_t target_addr = get_location_app();
    // App 1 (0x08004000) -> Page 8
    // App 2 (0x08012000) -> Page 36
    uint8_t target_page_start = (target_addr == ADDRESS_FLAG_APP2) ? 36 : 8;
    
    // Calculate required pages based on firmware size (2KB per page)
    uint8_t num_pages = (uint8_t)((incoming_total_bytes + 2047) / 2048);
    
    for (uint8_t page = target_page_start; page < (uint8_t)(target_page_start + num_pages); page++)
    {
        __disable_irq();
        ErasePage(page);
        __enable_irq();
    }
}

static void program_ota_firmware(uint8_t *firmware_buf, uint8_t data_length)
{
    uint32_t target_addr = get_location_app() + current_flash_addr;
    uint16_t double_words_count = (uint16_t)((data_length + 7U) / 8U);
    uint64_t padded_data[16];

    for (uint16_t i = 0; i < double_words_count; i++)
    {
        padded_data[i] = 0xFFFFFFFFFFFFFFFFULL;
    }
    memcpy(padded_data, firmware_buf, data_length);

    Program_Flash((void *)target_addr, padded_data, double_words_count);
    current_flash_addr += data_length;
}

static void Send_OTA_Response(uint16_t seq_num, uint8_t status)
{
    OTA_Respose_t res;
    res.Header = OTA_HEADER;
    res.Seq_num = seq_num;
    res.CommandId = status;
    res.CRC32 = Hardware_CRC_Calculate((uint8_t *)&res, 5);

    UART_Transmit_multi((char *)&res, sizeof(OTA_Respose_t));
}

void OTA_Process_Buffer(uint8_t *rx_buffer)
{
    OTA_Packet_t *packed = (OTA_Packet_t *)rx_buffer;

    if (packed->Header != OTA_HEADER)
    {
        Send_OTA_Response(expected_seq, OTA_START);
        return;
    }

    uint32_t crc = Hardware_CRC_Calculate((uint8_t *)packed, sizeof(OTA_Packet_t) - 4);

    if (crc != packed->CRC32)
    {
        Send_OTA_Response(expected_seq, OTA_NACK);
        return;
    }

    if (packed->CommandId == OTA_START)
    {

        /*4 byte version | 4 byte Total Byte | 2 byte total frame | 4 byte crc của file bin */
        incoming_version = *((uint32_t *)(&packed->Payload[0]));
        incoming_total_bytes = *((uint32_t *)(&packed->Payload[4]));
        incoming_total_frames = *((uint16_t *)(&packed->Payload[8]));
        incoming_global_crc = *((uint32_t *)(&packed->Payload[10]));

        determent_erase();

        current_flash_addr = 0;
        expected_seq = 0;
        Send_OTA_Response(expected_seq, OTA_ACK);
        expected_seq++;
        return;
    }

    switch (packed->CommandId)
    {
    case OTA_DATA:
    {
        uint8_t *pData = (uint8_t *)packed->Payload;
        program_ota_firmware(pData, packed->Length);
        Send_OTA_Response(expected_seq, OTA_ACK);
        expected_seq++;
        break;
    }

    case OTA_END:
    {
        uint32_t target_addr = get_location_app();
        uint32_t bin_crc = Calculate_Flash_CRC(target_addr, incoming_total_bytes);
        if (bin_crc == incoming_global_crc)
        {
            Send_OTA_Response(expected_seq, OTA_ACK);
            Write_OtaShared();
            Jump_To_App(target_addr);
            // reset chip
            AIRCR |= (0x5FA << 16) | (1U << 2);
        }
        else
        {
            // UART_Transmit_multi((char *)&res, sizeof(OTA_Respose_t));
            Send_OTA_Response(expected_seq, OTA_NACK);
        }

        break;
    }
    default:
        break;
    }
}
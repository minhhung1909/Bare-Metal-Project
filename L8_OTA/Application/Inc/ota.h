#ifndef OTA_H
#define OTA_H

#include <stdint.h>

#define OTA_MAGIC 0xDEADBEEF
#define MAX_FIRMWARE_SIZE 16384  // 16KB per receive

typedef struct {
    uint32_t magic;
    uint16_t firmware_len;
    uint16_t reserved;
    uint32_t firmware_crc;
} __attribute__((packed)) OTA_Header_t;

// Receive OTA packet header + firmware via polling UART
// Returns 0 on success, error code on failure
uint8_t receive_ota_packet(uint8_t *firmware_buf, uint16_t *out_len, uint32_t *out_crc);

// Program firmware to opposite slot with CRC validation
void program_ota_firmware(uint8_t *firmware_buf, uint16_t firmware_len, uint32_t firmware_crc);

#endif /* OTA_H */
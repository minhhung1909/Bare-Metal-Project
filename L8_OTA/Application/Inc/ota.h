#ifndef OTA_H
#define OTA_H

#include <stdint.h>

#define MAX_SIZE_BUFF   138

#define ADDRESS_FLAG_APP1           0x08004000U
#define ADDRESS_FLAG_APP2           0x08012000U

#define OTA_HEADER      0xA5
#define OTA_ACK         0x06
#define OTA_NACK        0x15

typedef enum
{
    /* 4 byte version | 4 byte Total Byte | 2 byte total frame | 4 byte crc của file bin*/
    OTA_START = 0x01,
    OTA_DATA = 0x02,
    OTA_END = 0x03
} command_id_t;

// typedef struct
// {
//     uint16_t Header;
//     uint16_t length;
//     command_id_t CommandId;
//     uint16_t reserved;
//     uint32_t firmware_crc;
// } __attribute__((packed)) OTA_Header_t;

typedef struct
{
    uint16_t Header;            //
    uint16_t Seq_num;           // 1
    uint8_t CommandId;          //
    uint8_t Length;             //  1
    uint8_t Payload[128];       //  32
    uint32_t CRC32;             // 1
} __attribute__((packed)) OTA_Packet_t;

typedef struct
{
    uint16_t Header;
    uint16_t Seq_num;
    uint8_t CommandId;
    uint32_t CRC32;
} __attribute__((packed)) OTA_Respose_t;

void OTA_Process_Buffer (uint8_t *rx_buffer);

#endif /* OTA_H */
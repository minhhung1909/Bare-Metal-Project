#ifndef OTA_H
#define OTA_H

#include <stdint.h>

typedef struct {
    uint32_t Magic_Word;
    uint32_t Active_App;       // Đang chạy App nào? (1 hoặc 2)
    
    uint32_t App1_Version;
    uint32_t App1_Size;
    uint32_t App1_CRC;
    
    uint32_t App2_Version;     
    uint32_t App2_Size;
    uint32_t App2_CRC;
    
    uint32_t Update_Status;    // Trạng thái: 0=Bình thường, 1=Có bản update mới cần test
    uint32_t Padding;
} OTA_Header_t;

#endif OTA_H
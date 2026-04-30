#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    uint32_t Magic_Word;      // Chữ ký xác thực (0xDEADBEEF)
    
    // --- 2. TRẠNG THÁI BOOT ---
    uint32_t Active_App;      // Đang chạy App nào? (1 hoặc 2)
    uint32_t Update_State;    // Trạng thái cập nhật (NORMAL = 0, PENDING = 1, SUCCESS = 2)
    
    // --- 3. BẢO VỆ HỆ THỐNG (ANTI-BRICK) ---
    uint32_t Retry_Count;     // Số lần Bootloader cố gắng khởi động App mới nhưng thất bại
    
    // --- 4. THÔNG TIN APP 1 ---
    uint32_t App1_Version;    // Phiên bản App 1 (VD: 10001)
    uint32_t App1_Size;       // Dung lượng App 1 (Bytes)
    uint32_t App1_CRC32;      // Mã toàn vẹn của App 1
    
    // --- 5. THÔNG TIN APP 2 ---
    uint32_t App2_Version;    // Phiên bản App 2
    uint32_t App2_Size;       // Dung lượng App 2
    uint32_t App2_CRC32;      // Mã toàn vẹn của App 2
    
    // Độn thêm cho đủ bội số của 8 byte
    uint32_t Reserved[2];     
} OTA_Shared_Config_t;

#endif
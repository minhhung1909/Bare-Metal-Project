#ifndef CONFIG_H
#define CONFIG_H

#define FLAG_ADDR 0x0801E800 // page 61

typedef struct {
    uint32_t Magic_Word;      // Sign for authorize (0xDEADBEEF)
    
    // --- BOOT STATE ---
    uint32_t Active_App;      // App running? (1 or 2)
    uint32_t Update_State;    // State update (NORMAL = 0, PENDING = 1, SUCCESS = 2)
    
    // --- ANTI-BRICK ---
    uint32_t Retry_Count;     // Bootloader times but fail
    
    // --- info app 1 ---
    uint32_t App1_Version;    // Version 1 (ex: 10 00 01 00)
    uint32_t App1_Size;       // size App 1 (Bytes)
    uint32_t App1_CRC32;
    
    // --- 5. THÔNG TIN APP 2 ---
    uint32_t App2_Version;    // Version 1 (ex: 10 00 01 01)
    uint32_t App2_Size;       // Size App 2
    uint32_t App2_CRC32;

    // padding
    uint32_t Reserved[2];     
} OTA_Shared_Config_t;

#endif
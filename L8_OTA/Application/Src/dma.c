#include "dma.h"

void DMA_init(uint8_t *data, uint16_t size)
{
    /* Theo những dòng arm cũ hơn thì channel của DMA với các ngoại vi
     * Được fix cứng nhưng dòng G4 này mới hơn có mux của DMA nên không cần care channel
     * quan trọng là vẫn phải define 1 channel nào đó thì ở đây chọn channel 1
     */

    // Cấp clock AHB1ENR bit 0 là DMA1, bit 2 là DMAMUX
    RCC_AHB1ENR |= (1U << 0) | (1U << 2);

    DMA_CCR1 &= ~(1U << 0);

    // 0: là ID UartRX: 26
    DMAMUX_C0CR &= ~(0x7FU << 0);
    DMAMUX_C0CR |= (26U << 0);

    DMA_CPAR1 = (uint32_t)&USART2_RDR; // Lưu địa chỉ USART2_RDR cho DMA để lấy dữ liệu
    DMA_CMAR1 = (uint32_t)data;        // Lấy địa chỉ để lưu dữ liệu vào mảng

    // Number of data to transfe
    DMA_CNDTR1 = size;

    // 7 Memory increment mode
    // 5 Circular mode
    // 1 Transfer complete interrupt enable
    // 0 Channel enable
    // MSIZE: 8 và PSIZE: 10 mặc định bằng 0 tức là 8-bit, đúng với kiểu 'char' nên không cấu hình.
    DMA_CCR1 |= (1U << 7 | 1U << 1 | 1U << 0);

    // DMA enable receiver
    USART2_CR3 |= (1U << 6);

    // Enable NVIC
    NVIC_ISER0 |= (1U << 11);
}
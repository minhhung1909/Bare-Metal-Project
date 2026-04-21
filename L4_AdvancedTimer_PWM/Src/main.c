/* Bài này học TIM1 thì xem trước TIM 1 ở Bus APB2
* 
*
*/

#include "Systick.h"
#include <stdint.h>
#include <stdbool.h>


#define ARR_OFSET               0x2C
#define PSC_OFSET               0x28
#define CCR1_OFFSET             0x34
#define CCMR1_OFFSET            0x18
#define CCER1_OFFSET            0x20
#define BDTR_OFFSET             0x44
#define CR1_OFFSET              0x00
#define MODER_OFFSET            0x00

#define RCC_BASE_ADRR           0x40021000
#define PORTA_BASE              0x48000000
#define PORTC_BASE              0x48000800
#define TIM6_BASE_ADRR          0x40001000
#define TIM1_BASE_ADRR          0x40012C00 

#define TIM1_ARR_ADRR           (*(volatile unsigned int *) (TIM1_BASE_ADRR + ARR_OFSET))
#define TIM1_PSC_ADRR           (*(volatile unsigned int *) (TIM1_BASE_ADRR + PSC_OFSET))
#define TIM1_CCR1_ADRR          (*(volatile unsigned int *) (TIM1_BASE_ADRR + CCR1_OFFSET))
#define TIM1_CCMR1_ADRR         (*(volatile unsigned int *) (TIM1_BASE_ADRR + CCMR1_OFFSET))
#define TIM1_CCER_ADRR          (*(volatile unsigned int *) (TIM1_BASE_ADRR + CCER1_OFFSET))
#define TIM1_BDRT_ADRR          (*(volatile unsigned int *) (TIM1_BASE_ADRR + BDTR_OFFSET))
#define TIM1_CR1_ADRR           (*(volatile unsigned int *) (TIM1_BASE_ADRR + CR1_OFFSET))

#define GPIOA_MODER             (*(volatile unsigned int *)(PORTA_BASE + MODER_OFFSET))
#define GPIOA_AFRH              (*(volatile unsigned int *)(PORTA_BASE + 0x24))

#define RCC_APB2ENR             (*(volatile unsigned int *)(RCC_BASE_ADRR + 0x60))
#define RCC_AHB2ENR             (*(volatile unsigned int *)(RCC_BASE_ADRR + 0x4C))
#define GPIOC_MODER             (*(volatile unsigned int *)(PORTC_BASE + MODER_OFFSET))
#define GPIOC_ODR               (*(volatile unsigned int *)(PORTC_BASE + 0x14))
#define RCC_APB1ENR1            (*(volatile unsigned int *)(RCC_BASE_ADRR + 0x58))

#define TIM6_PSC_ADRR           (*(volatile unsigned int *) (TIM6_BASE_ADRR + PSC_OFSET))
#define TIM6_ARR_ADRR           (*(volatile unsigned int *) (TIM6_BASE_ADRR + ARR_OFSET))
#define TIM6_DMA_INT_ENB        (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x0C))
#define TIM6_CONTROL_1          (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x00))
#define TIM6_SR                 (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x10))

#define NVIC_ISER1              (*(volatile unsigned int *) (0xE000E104))

#define SYSCLK_HZ               (16000000U)

#define RCC_ABP2_TIM1           0x40012C00

void init_TIM6(void){
    RCC_APB1ENR1 |= (1U << 4);
    TIM6_PSC_ADRR = 1600-1;
    TIM6_ARR_ADRR = 5000-1;
    TIM6_DMA_INT_ENB |= (1U << 0);

    NVIC_ISER1 |= (1U << 22);   //Ngắt 54 nằm ở thanh ghi ISER1 (vị trí bit: 54 - 32 = 22)

    TIM6_CONTROL_1 |= (1u << 0);

}

void init_led(void){
    RCC_AHB2ENR |= (1U << 2);

    GPIOC_MODER &= ~( 3U << (6*2) );
    GPIOC_MODER |= ( 1U << (6*2) );
}

void TIM6_DAC_IRQHandler(void){
    if (TIM6_SR & (1U << 0)) {

        TIM6_SR &= ~(1U << 0);

        GPIOC_ODR ^= (1U << 6);
    }
}

void init_PWM_PA8(void)
{
    /* Bật Nguồn*/
    RCC_AHB2ENR |= (0x1U << 0);   // Bật nguồn cho Port A (Chứa chân PA8)
    RCC_APB2ENR |= (0x1U << 11);  // Bật nguồn cho TIM1 (Nằm trên bus APB2)


    GPIOA_MODER &= ~(0b11U << (8 * 2)); 
    // Chuyển PA8 sang chế độ Alternate Function (10)
    GPIOA_MODER |=  (0b10U << (8 * 2));

    GPIOA_AFRH  &= ~(0b1111U << 0);
    GPIOA_AFRH  |=  (0b0110U << 0);       // Nạp số 6 (AF6)

    /* Cấu hình tần số pwm dựa vào ftim, PSC, ARR.
    Vì cấu hình tần số chip là 16MHZ -> ABP2 cũng là 16MHZ 
    Mà muốn fPWM là 50HZ thì Lúc này PSC = 160-1 và ARR = 2000-1
    */

    TIM1_ARR_ADRR = 2000-1;
    TIM1_PSC_ADRR = 160-1;

    /* Tiếp theo là cấu hình DutyCycle = (CCRx / (ARR +1 )) * 100 đang ví dụ thử 10% 
    Nếu muốn có 10% duty thì CRR1 = 200-1
    */
    TIM1_CCR1_ADRR = 200;

    /* Có 2 mode chính của so sánh để tạo ra mức cao hoặc mức thấp. Mode 1 thì channel 1 activate long ngược là là inactivate (TIMx_CNT < TIMx_CCR1 thì). 
    Mode 2 thì đảo lại thôi channel 1 activate long TIMx_CNT > TIMx_CCR1
    Thì dưới đây là dùng Mode 1 nên bật 2 bit là 0110
    */
    TIM1_CCMR1_ADRR &= ~(0b111U << 4 | 1U << 16);
    TIM1_CCMR1_ADRR |= (0b110U << 4); 

    TIM1_CCMR1_ADRR |= (0b1U << 3); // OC1PE


    /* Tiếp theo là enable bit CC1E ở thanh ghi CCER để xuất xung*/
    TIM1_CCER_ADRR |= (0b1U << 0);

    TIM1_BDRT_ADRR |= (0b1U << 15);

    TIM1_CR1_ADRR |= (0b1U << 0); // CEN = 1: Bắt đầu đếm!
}

int main(void) {
    // init_TIM6();
    // init_led();
    init_PWM_PA8();
    for (;;) {

    }
    return 0;
}
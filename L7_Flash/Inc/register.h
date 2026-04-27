#ifndef REGISTER_H
#define REGISTER_H

#define ARR_OFSET               0x2C
#define PSC_OFSET               0x28
#define CCR1_OFFSET             0x34
#define CCMR1_OFFSET            0x18
#define CCER1_OFFSET            0x20
#define BDTR_OFFSET             0x44
#define CR1_OFFSET              0x00
#define MODER_OFFSET            0x00
#define RCC_AHB2ENR_OFFSET      0x4C
#define RCC_AHB1ENR_OFFSET      0x48     
#define GPIOx_AFRL_OFFSET       0x20
#define GPIOx_AFRH_OFFSET       0x24
#define RCC_APB1ENR1_OFFSET     0x58

#define RCC_BASE_ADRR           0x40021000
#define PORTA_BASE              0x48000000
#define PORTB_BASE              0x48000400
#define PORTC_BASE              0x48000800
#define TIM6_BASE_ADRR          0x40001000
#define TIM1_BASE_ADRR          0x40012C00 
#define USART2_BASE_ADRR        0x40004400
#define RCC_AHB1_BASE_ARR       0x40020000
#define FLASH_INTERFACE_ARR     0x40022000


#define TIM1_ARR_ADRR           (*(volatile unsigned int *) (TIM1_BASE_ADRR + ARR_OFSET))
#define TIM1_PSC_ADRR           (*(volatile unsigned int *) (TIM1_BASE_ADRR + PSC_OFSET))
#define TIM1_CCR1_ADRR          (*(volatile unsigned int *) (TIM1_BASE_ADRR + CCR1_OFFSET))
#define TIM1_CCMR1_ADRR         (*(volatile unsigned int *) (TIM1_BASE_ADRR + CCMR1_OFFSET))
#define TIM1_CCER_ADRR          (*(volatile unsigned int *) (TIM1_BASE_ADRR + CCER1_OFFSET))
#define TIM1_BDRT_ADRR          (*(volatile unsigned int *) (TIM1_BASE_ADRR + BDTR_OFFSET))
#define TIM1_CR1_ADRR           (*(volatile unsigned int *) (TIM1_BASE_ADRR + CR1_OFFSET))

#define GPIOA_MODER             (*(volatile unsigned int *)(PORTA_BASE + MODER_OFFSET))
#define GPIOA_AFRH              (*(volatile unsigned int *)(PORTA_BASE + GPIOx_AFRH_OFFSET))
#define GPIOA_AFRL              (*(volatile unsigned int *)(PORTA_BASE + GPIOx_AFRL_OFFSET))


#define RCC_APB2ENR             (*(volatile unsigned int *)(RCC_BASE_ADRR + 0x60))
#define RCC_AHB2ENR             (*(volatile unsigned int *)(RCC_BASE_ADRR + RCC_AHB2ENR_OFFSET))
#define RCC_AHB1ENR             (*(volatile unsigned int *)(RCC_BASE_ADRR + RCC_AHB1ENR_OFFSET))
#define GPIOC_MODER             (*(volatile unsigned int *)(PORTC_BASE + MODER_OFFSET))
#define GPIOC_ODR               (*(volatile unsigned int *)(PORTC_BASE + 0x14))
#define RCC_APB1ENR1            (*(volatile unsigned int *)(RCC_BASE_ADRR + RCC_APB1ENR1_OFFSET))


#define TIM6_PSC_ADRR           (*(volatile unsigned int *) (TIM6_BASE_ADRR + PSC_OFSET))
#define TIM6_ARR_ADRR           (*(volatile unsigned int *) (TIM6_BASE_ADRR + ARR_OFSET))
#define TIM6_DMA_INT_ENB        (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x0C))
#define TIM6_CONTROL_1          (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x00))
#define TIM6_SR                 (*(volatile unsigned int *) (TIM6_BASE_ADRR + 0x10))

#define NVIC_ISER0              (*(volatile unsigned int *) (0xE000E100))
#define NVIC_ISER1              (*(volatile unsigned int *) (0xE000E104))


#define SYSCLK_HZ               (16000000U)

#define RCC_ABP2_TIM1           0x40012C00

/* uart */
#define USART2_BRR              (*(volatile unsigned int *) (USART2_BASE_ADRR + 0x0C))
#define USART2_TDR              (*(volatile unsigned int *) (USART2_BASE_ADRR + 0x28))
#define USART2_ISR              (*(volatile unsigned int *) (USART2_BASE_ADRR + 0x1C))
#define USART2_RDR              (*(volatile unsigned int *) (USART2_BASE_ADRR + 0x24))
#define USART2_CR1              (*(volatile unsigned int *) (USART2_BASE_ADRR + 0x00))
#define USART2_CR3              (*(volatile unsigned int *) (USART2_BASE_ADRR + 0x08))
#define USART2_ICR              (*(volatile unsigned int *) (USART2_BASE_ADRR + 0x20))

/* DMA MUX*/
#define DMAMUX_BASE            0x40020800
#define DMAMUX_C0CR             (*(volatile unsigned int *) (DMAMUX_BASE + 0x00 + 0x04 * 0) ) 

/* DMA */
#define DMA1_BASE_ARR           RCC_AHB1_BASE_ARR + 0x00000000U
#define DMA_CPAR1               (*(volatile unsigned int *) (DMA1_BASE_ARR + (0x10 + 0x14 * (1 - 1)) ) ) // Chọn channel 1
#define DMA_CMAR1               (*(volatile unsigned int *) (DMA1_BASE_ARR + (0x14 + 0x14 * (1 - 1)) ) )
#define DMA_CNDTR1              (*(volatile unsigned int *) (DMA1_BASE_ARR + (0x0C + 0x14 * (1 - 1)) ) )
#define DMA_CCR1                (*(volatile unsigned int *) (DMA1_BASE_ARR + (0x08 + 0x14 * (1 - 1)) ) )
#define DMA1_IFCR               (*(volatile unsigned int *) (DMA1_BASE_ARR + 0x04))

/*Flash*/
#define FLASH_SR                (*(volatile unsigned int *) (FLASH_INTERFACE_ARR + 0x10))
#define FLASH_CR                (*(volatile unsigned int *) (FLASH_INTERFACE_ARR + 0x14))
#define FLASH_KEYR              (*(volatile unsigned int *) (FLASH_INTERFACE_ARR + 0x08))

#endif
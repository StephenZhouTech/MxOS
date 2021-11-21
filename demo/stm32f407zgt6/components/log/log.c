#include "log.h"
#include <stdint.h>
#include "stm32f4xx_conf.h"

#define EASY_PRINTF     1

#if EASY_PRINTF
#pragma import(__use_no_semihosting)
struct __FILE
{ 
    int handle;
}; 

FILE __stdout;
void _sys_exit(int x)
{ 
    x = x;
} 

int fputc(int ch, FILE *f)
{   
    while((USART1->SR&0x40)==0);
    USART1->DR = (u8) ch;
    return ch;
}
#endif

void LogInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // Enable GPIOA clock gate
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
    // Enable USART1 clock gate
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);

    // Pinmux for USART1 GPIO-A9 and GPIO-A10
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);

    // Configure IO for USART1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA,&GPIO_InitStructure);

    // USART1 Init
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);

    //USART_ClearFlag(USART1, USART_FLAG_TC);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void USART1_IRQHandler(void)
{
//    uint8_t Res;

//    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
//    {
//        Res = USART_ReceiveData(USART1);
//    }
}

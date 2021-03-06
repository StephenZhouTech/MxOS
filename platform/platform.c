/*
 * MxOS Kernel V0.1
 * Copyright (C) 2020 StephenZhou.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 1 tab == 4 spaces!
 */

/* 
 * Initial the platform
 * Just like Console, Clock(PLL), PMU, Wake up source
 */
#include "platform.h"
#include "Log.h"
#include "led.h"
#include "os_configs.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx.h"

void PlatforUartInit(void)
{
    LogInit();
}

void PlatformInit(void)
{
    PlatforUartInit();

    BspLedInit();
}

#if CONFIG_USE_SHELL
void PlatformUartSendDataPolling(const char ch)
{
    while((USART1->SR&0x40)==0);
    USART1->DR = (u8) ch;
}

signed char PlatformUartRecvDataPolling(char *ch)
{
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
    {
        *ch = USART_ReceiveData(USART1);
        return 0;
    }
    return -1;
}
#endif // CONFIG_USE_SHELL

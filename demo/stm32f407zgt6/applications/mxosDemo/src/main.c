#include "stm32f4xx.h"
#include "delay.h"
#include "log.h"
#include "led.h"
#include "lcd_st7735s.h"
#include "lcd_st7735s_api.h"

#define UART_DEBUG      1
#define USE_LED         1

int main(void)
{
    uint32_t counter = 0;

#if UART_DEBUG
    LogInit();
#endif

#if USE_LED
    BspLedInit();
#endif

    delay_init(168);

    LOG_DEBUG("**********************************************************");
    LOG_DEBUG("************** Board Test Project Running ****************");
    LOG_DEBUG("**********************************************************");
    LOG_INFO("log Ready...");
    LOG_INFO("led Ready...");

    LCD_ST7735S_Init();
    LOG_INFO("LCD Ready...");

    //LCD_Fill(0, 0, 128, 15, BLUE);
    LCD_ShowString(1,  0, "0. LED Ready...", BLACK, WHITE, 12, 0);
    LCD_ShowString(1, 16, "1. LOG Ready...", BLACK, WHITE, 12, 0);
    LCD_ShowString(1, 32, "2. LCD Ready...", BLACK, WHITE, 12, 0);

    while(1)
    {
        LOG_DEBUG("System Running Counter:%d",counter++);
        delay_ms(100);
        BspLEDOn(LED_2 | LED_3 | LED_4);
        delay_ms(100);
        BspLEDOff(LED_2 | LED_3 | LED_4);
    }
}

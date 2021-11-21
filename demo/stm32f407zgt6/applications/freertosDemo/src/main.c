#include "stm32f4xx.h"
#include "delay.h"
#include "log.h"
#include "led.h"
#include "lcd_st7735s.h"
#include "lcd_st7735s_api.h"

#include "FreeRTOS.h"
#include "task.h"

#define UART_DEBUG      1
#define USE_LED         1


void vTaskFunction1( void * pvParameters )
{
    static uint32_t cnt = 0;
    for( ;; )
    {
        if(cnt == 0)
        {
            BspLEDOn(LED_2 | LED_3 | LED_4);
            cnt = 1;
        }
        else
        {
            BspLEDOff(LED_2 | LED_3 | LED_4);
            cnt = 0;
        }
        vTaskDelay( 1000 );
    }
}

void vTaskFunction2( void * pvParameters )
{
    static uint32_t counter = 0;
    for( ;; )
    {
        LOG_DEBUG("System Running Counter:%d",counter++);
        vTaskDelay( 1000 );
    }
}

int main(void)
{

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
    LCD_ShowString(1,  0, "0. LED1 Ready...", BLACK, WHITE, 12, 0);
    LCD_ShowString(1, 16, "1. LOG1 Ready...", BLACK, WHITE, 12, 0);
    LCD_ShowString(1, 32, "2. LCD1 Ready...", BLACK, WHITE, 12, 0);

    xTaskCreate( vTaskFunction1, "LED", 1024, NULL, 1, NULL );
    xTaskCreate( vTaskFunction2, "Print", 1024, NULL, 1, NULL );
    vTaskStartScheduler();

    // NEVER GO HERE
    while(1);
}

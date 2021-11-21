#include "stm32f4xx.h"
#include "delay.h"
#include "log.h"
#include "led.h"
#include "os_mem.h"

int main(void)
{
    uint32_t counter = 0;

    LogInit();

    BspLedInit();

    delay_init(168);

    LOG_DEBUG("**********************************************************");
    LOG_DEBUG("************** MxOS Project Running ....****************");
    LOG_DEBUG("**********************************************************");

    while(1)
    {
        LOG_DEBUG("System Running Counter:%d",counter++);
        delay_ms(100);
        BspLEDOn(LED_2 | LED_3 | LED_4);
        delay_ms(100);
        BspLEDOff(LED_2 | LED_3 | LED_4);
    }
}

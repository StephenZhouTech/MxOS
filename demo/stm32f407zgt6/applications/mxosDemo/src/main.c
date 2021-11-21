#include "stm32f4xx.h"
#include "delay.h"
#include "log.h"
#include "led.h"
#include "os_mem.h"
#include "os_list.h"

void *m1 = OS_NULL;

typedef struct people {
    char p1;
    int p2;
    int p3;

} people_t;

int main(void)
{
    uint32_t counter = 0;

    LogInit();

    BspLedInit();

    delay_init(168);

    m1 = OS_API_Malloc(1024);

    LOG_DEBUG("**********************************************************");
    LOG_DEBUG("************** MxOS Project Running ....****************");
    LOG_DEBUG("**********************************************************");
    LOG_DEBUG("Malloc addr = 0x%08x", (uint32_t)m1);

    people_t peo = {'a', 1, 2};
    people_t *p_peo = NULL;
    char *p_ch = &peo.p1;

    p_peo = ContainerOf(p_ch, people_t, p1);
    LOG_DEBUG("p1 = %c", p_peo->p1);
    LOG_DEBUG("p2 = %d", p_peo->p2);
    LOG_DEBUG("p3 = %d", p_peo->p3);

    while(1)
    {
        //LOG_DEBUG("System Running Counter:%d",counter++);
        delay_ms(100);
        BspLEDOn(LED_2 | LED_3 | LED_4);
        delay_ms(100);
        BspLEDOff(LED_2 | LED_3 | LED_4);
    }
}

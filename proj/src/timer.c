#include <lcom/lcf.h>
#include <stdint.h>
#include "utils.h"
#include "timer.h"

extern int time_elapsed;
extern int hook_timer;

int (timer_subscribe_int)(uint8_t *bit_no) {
    hook_timer = TIMER0_IRQ;
    *bit_no = BIT(hook_timer);
    if(sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_timer) == 0){
        return 0;
    }
    return 1;
}

int (timer_unsubscribe_int)() {
    if(sys_irqrmpolicy(&hook_timer) == 0) return 0;
    return 1;
}

void (timer_int_handler)() {
    time_elapsed++;
}

void rtc_time(struct Date *real_date){

    uint32_t aux;

    sys_outb(RTC_ADDR_REG, RTC_DAYOFMONTH);
    sys_inb(RTC_DATA_REG, &aux);
    real_date->day = bcd_to_decimal( aux);

    sys_outb(RTC_ADDR_REG, RTC_MONTH);
    sys_inb(RTC_DATA_REG, &aux);
    real_date->month = bcd_to_decimal( aux);

    sys_outb(RTC_ADDR_REG, RTC_YEAR);
    sys_inb(RTC_DATA_REG, &aux);
    real_date->year = bcd_to_decimal(aux) + MILLENIUM;

    sys_outb(RTC_ADDR_REG, RTC_SECONDS);
    sys_inb(RTC_DATA_REG, &aux);
    real_date->second = bcd_to_decimal( aux);


    sys_outb(RTC_ADDR_REG, RTC_MINUTES);
    sys_inb(RTC_DATA_REG, &aux);
    real_date->minute = bcd_to_decimal( aux);

    sys_outb(RTC_ADDR_REG, RTC_HOURS);
    sys_inb(RTC_DATA_REG, &aux);
    real_date->hour = bcd_to_decimal( aux);

}

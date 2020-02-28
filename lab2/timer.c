#include <lcom/lcf.h>
#include <lcom/timer.h>
#include <stdint.h>
#include "i8254.h"
#include "lab2.h"
#include "timer.h"


int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
    uint16_t counter = (uint16_t) (TIMER_FREQ/freq);
    uint8_t lsb, msb;
    util_get_LSB(counter, &lsb);
    util_get_MSB(counter, &msb);
    uint8_t control_word;
    timer_get_conf(timer, &control_word);
    control_word &= TIMER_4LSBs;
    control_word |= TIMER_LSB_MSB;

    if (timer == 0){
        sys_outb(TIMER_CTRL, control_word);
        sys_outb(TIMER_0, lsb);
        sys_outb(TIMER_0, msb);
        return 0;
    } else if (timer == 1){
        control_word |= TIMER_SEL1;
        sys_outb(TIMER_CTRL, control_word);
        sys_outb(TIMER_1, lsb);
        sys_outb(TIMER_1, msb);
        return 0;
    } else if (timer == 2){
        control_word |= TIMER_SEL2;
        sys_outb(TIMER_CTRL, control_word);
        sys_outb(TIMER_2, lsb);
        sys_outb(TIMER_2, msb);
        return 0;
    } else {
        printf("Timer does not exist!\n");
        return 1;
    }
}

int (timer_subscribe_int)(uint8_t *bit_no) {
    hook_id = TIMER0_IRQ;
    *bit_no = hook_id;
    if(sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) == 0){
        return 0;
    }
    return 1;
}

int (timer_unsubscribe_int)() {
    if(sys_irqrmpolicy(&hook_id) == 0) return 0;
    return 1;
}

void (timer_int_handler)() {
    time_elapsed++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
    uint32_t rbc = TIMER_RB_CMD | BIT(5);
    int port = 0;
    if(timer&TIMER_READ1){
        rbc=rbc | BIT(2);
        port=TIMER_1;
    }
    else if(timer&TIMER_READ2){
        rbc=rbc | BIT(3);
        port=TIMER_2;
    }
    else if (timer!=0x0){
        printf("Timer does not exist\n");
        return 1;
    }
    else{
        rbc=rbc | BIT(1);
        port=TIMER_0;
    }
    sys_outb(TIMER_CTRL,rbc);

    if(util_sys_inb(port,st)){
        return 1;
    }
    return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {
    union timer_status_field_val displayer;
    if (field == tsf_all){
        displayer.byte = st;
    } else if (field == tsf_initial){
        if ((st & TIMER_LSB_MSB) == TIMER_LSB_MSB){
            displayer.in_mode = MSB_after_LSB;
        } else if (st & TIMER_LSB){
            displayer.in_mode = LSB_only;
        } else if (st & TIMER_MSB){
            displayer.in_mode = MSB_only;
        } else {
            displayer.in_mode = INVAL_val;
        }
    } else if (field == tsf_mode){
        if ((st & TIMER_SQR_WAVE) == TIMER_SQR_WAVE){
            displayer.count_mode = 3;
        }  else if (st & TIMER_RATE_GEN) {
            displayer.count_mode = 2;
        }  else if ((st & TIMER_MODE5) == TIMER_MODE5){
            displayer.count_mode = 5;
        } else if (st & TIMER_MODE1){
            displayer.count_mode = 1;
        } else if (st & TIMER_MODE4){
            displayer.count_mode = 4;
        } else {
            displayer.count_mode = 0;
        }
    } else {
        if (st & TIMER_BCD) {
            displayer.bcd = true;
        } else {
            displayer.bcd = false;
        }
    }
    timer_print_config(timer, field, displayer);
    return 0;
}

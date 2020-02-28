#include <lcom/lcf.h>
#include <lcom/lab2.h>
#include <timer.h>
#include <i8254.h>
#include <stdbool.h>
#include <stdint.h>
#include "lab2.h"

int time_elapsed = 0;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
    if (timer < 0 || timer > 2){
        printf("Invalid Timer!! (must be between 0 and 2)\n");
        return 1;
    }
    if (field != tsf_all && field != tsf_initial && field != tsf_mode && field != tsf_base){
        printf("Invalid field!! (must be all, init, mode or base)\n");
        return 1;
    }
    uint8_t *status = malloc(sizeof *status);
    timer_get_conf(timer, status);
    timer_display_conf(timer, *status, field);
    free(status);
    return 0;
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
    if (timer < 0 || timer > 2){
        printf("Invalid Timer!! (must be between 0 and 2)\n");
        return 1;
    }
    if (freq < 19){ // the divisor has to be 65535 (2¹⁶ -1), so freq has to be higher than 18 (1193182 / 65535 = 18.21)
        printf("Frequency too low!!\n");
        return 1;
    }
    if (freq > TIMER_FREQ){ // the divisor has to be >0
        printf("Frequency too high!!\n");
        return 1;
    }

    timer_set_frequency(timer, freq);

    return 0;
}

int(timer_test_int)(uint8_t time) {
    if (time < 1) { // negative time does not exist; 0 as an input does nothing
        printf("Time too low!!\n");
        return 1;
    }
    uint8_t bit_no;
    timer_subscribe_int(&bit_no);
    uint32_t irq_set = BIT(bit_no);
    int ipc_status;
    message msg;
    while(time_elapsed/60 < time) { // 60Hz is the standard frequency of the os and it will not change
        int r;
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set) {
                        timer_int_handler();
                        if(time_elapsed%60==0) {
                            timer_print_elapsed_time();
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    timer_unsubscribe_int();
    return 0;
}

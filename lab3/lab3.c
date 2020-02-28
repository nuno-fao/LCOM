#include <lcom/lcf.h>
#include <lcom/lab3.h>
#include <stdbool.h>
#include <stdint.h>
#include "utils.h"

uint32_t inbcalls;
uint8_t code;
int hook_kbd;
int hook_timer;
int time_elapsed;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int (kbd_test_scan)() {
    uint8_t size = 1;
    bool two_B = false;
    bool make;
    code = 0;
    uint8_t code_bytes[2];
    int ipc_status;
    inbcalls=0;
    message msg;


    uint8_t bit_no=1;
    if(kbd_subscribe(&bit_no) == 1){
        printf("Couldn't subscribe keyboard!\n");
        return 1;
    };
    uint32_t irq_set = BIT(bit_no);

    while(code != ESC_BC) {
        int r;
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set) {
                        kbc_ih();
                        if (two_B){
                            code_bytes[1] = code;
                            two_B = false;
                        } else if (code == TWO_BYTE_CODE){
                            code_bytes[0] = code;
                            size = 2;
                            two_B = true;
                        } else {
                            size = 1;
                            code_bytes[0] = code;
                            two_B = false;
                        }
                        if (!two_B){
                            if(MAKE&code) make=false;
                            else make=true;
                            kbd_print_scancode(make, size, code_bytes);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    kbd_print_no_sysinb(inbcalls);
    if(unsubscribe_kbd(&bit_no)){
        printf("Couldn't unsubscribe keyboard!\n");
        return 1;
    }

    return 0;
}

int(kbd_test_poll)() {
    uint8_t size = 1, status;
    bool make = false;
    uint8_t code_bytes[2];
    inbcalls = 0;


    while(code != ESC_BC){
        if(new_sys_inb(STAT_REG, &status))return 1;

        if ((status & OBF) && !(status & AUX) && !(status & (PAR_ERR | TO_ERR))){
            if(new_sys_inb(OUT_BUF, &code))return 1;
            if(code == TWO_BYTE_CODE){
                size = 2;
                code_bytes[0] = code;

                if(new_sys_inb(OUT_BUF, &code))return 1;
                code_bytes[1] = code;
            } else {
                size = 1;
                code_bytes[0] = code;
            }
            if(code & MAKE){
                make = false;
            } else {
                make = true;
            }
            kbd_print_scancode(make, size, code_bytes);
        }
        tickdelay(micros_to_ticks(DELAY_US));

    }

    kbd_print_no_sysinb(inbcalls);

    if (enable_kbd()) {
        printf("Couldn't reenable keyboard!\n");
        return 1;
    }
    return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
    uint8_t size = 1;
    bool two_B = false;
    bool make;
    code = 0;
    uint8_t code_bytes[2];
    int ipc_status;
    inbcalls=0;
    time_elapsed = 0;
    message msg;

    if(n==0){return 1;}

    uint8_t bit_kbd=1;
    if(kbd_subscribe(&bit_kbd) == 1){
        printf("Couldn't subscribe keyboard!\n");
        return 1;
    };
    uint32_t irq_set_kbd = BIT(bit_kbd);

    uint8_t bit_timer=0;
    if(timer_subscribe_int(&bit_timer) == 1){
        printf("Couldn't subscribe timer!\n");
        return 1;
    };
    uint32_t irq_set_timer = BIT(bit_timer);

    while(time_elapsed/60 < n && code != ESC_BC) {
        int r;
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set_kbd) {
                        kbc_ih();
                        if (two_B){
                            code_bytes[1] = code;
                            two_B = false;
                        } else if (code == TWO_BYTE_CODE){
                            code_bytes[0] = code;
                            size = 2;
                            two_B = true;
                        } else {
                            size = 1;
                            code_bytes[0] = code;
                            two_B = false;
                        }
                        if (!two_B){
                            if(MAKE&code) make=false;
                            else make=true;
                            kbd_print_scancode(make, size, code_bytes);
                            time_elapsed = 0;
                        }
                    }
                    if (msg.m_notify.interrupts & irq_set_timer){
                        timer_int_handler();
                    }
                    break;
                default:
                    break;
            }
        }
    }

    kbd_print_no_sysinb(inbcalls);
    if(unsubscribe_kbd(&bit_kbd)){
        printf("Couldn't unsubscribe keyboard!\n");
        return 1;
    }
    if(timer_unsubscribe_int()){
        printf("Couldn't unsubscribe timer!\n");
        return 1;
    }

    return 0;
}

#include <lcom/lcf.h>
#include <stdint.h>
#include "utils.h"
#include "kbd.h"

extern uint8_t code;
extern int hook_kbd;

int (kbd_subscribe)(uint8_t *bit_no) {
    hook_kbd = KBD_IRQ;
    *bit_no = BIT(hook_kbd);
    if(sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_kbd) == 0){
        return 0;
    }
    return 1;
}

int (unsubscribe_kbd)() {
    if (sys_irqrmpolicy(&hook_kbd) != 0) {
        return 1;
    }
    return 0;
}

void (kbc_ih)(){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);
    if  (status & OBF) {
        if ((status & (PAR_ERR | TO_ERR))){
            return;
        }
        new_sys_inb(OUT_BUF, &code);
    }
}

int kbd_read_cmd(uint8_t *cmd){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);

    while(!(status & OBF)){
        tickdelay(micros_to_ticks(DELAY_US));
        new_sys_inb(STAT_REG, &status);
    }
    if ((status & (PAR_ERR | TO_ERR))){
        return 1;
    }
    if (new_sys_inb(OUT_BUF, cmd)) {
        return 1;
    }
    return 0;
}

int kbd_write_cmd(port_t port, uint8_t cmd){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);

    while(status & IBF){
        tickdelay(micros_to_ticks(DELAY_US));
        new_sys_inb(STAT_REG, &status);
    }
    if ((status & (PAR_ERR | TO_ERR))){
        return 1;
    }
    if (sys_outb(port, cmd)) {
        return 1;
    }
    return 0;
}

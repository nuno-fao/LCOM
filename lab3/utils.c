#include <lcom/lcf.h>
#include <stdint.h>
#include "utils.h"


extern uint8_t code;
extern int hook_kbd;
extern int hook_timer;
extern uint32_t inbcalls;
extern int time_elapsed;


int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
    *lsb = (uint8_t) val;
    return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
    val >>= 8;
    *msb = (uint8_t) val;
    return 0;
}

int new_sys_inb(port_t port, uint8_t *byte) {
	uint32_t byte32;
	if(sys_inb(port, &byte32)) {
        printf("Couldn't read from port %x!\n",port);
        return 1;
    }
	*byte = (uint8_t)byte32;
	inbcalls++;
	return 0;
}

int (kbd_subscribe)(uint8_t *bit_no) {
    hook_kbd = KBD_IRQ;
    *bit_no = hook_kbd;
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

int kbd_send_cmd_to_read(){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);

    while(status & IBF){
        tickdelay(micros_to_ticks(DELAY_US));
        new_sys_inb(STAT_REG, &status);
    }
    if (sys_outb(KBC_CMD_REG, R_CMD)) {
        return 1;
    }
    return 0;
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

int kbd_send_cmd_to_write(){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);

    while(status & IBF){
        tickdelay(micros_to_ticks(DELAY_US));
        new_sys_inb(STAT_REG, &status);
    }
    if ((status & (PAR_ERR | TO_ERR))){
        return 1;
    }
    if (sys_outb(KBC_CMD_REG, W_CMD)) {
        return 1;
    }
    return 0;
}

int kbd_write_cmd(uint8_t cmd){
    uint8_t status=0;
    new_sys_inb(STAT_REG, &status);

    while(status & IBF){
        tickdelay(micros_to_ticks(DELAY_US));
        new_sys_inb(STAT_REG, &status);
    }
    if ((status & (PAR_ERR | TO_ERR))){
        return 1;
    }
    if (sys_outb(OUT_BUF, cmd)) {
        return 1;
    }
    return 0;
}

int enable_kbd(){
    uint8_t cmd = 0;
    if (kbd_send_cmd_to_read()) return 1;    // sends command to read command byte
    if (kbd_read_cmd(&cmd)) return 1;     // reads command
    cmd |= BIT(0);
    if (kbd_send_cmd_to_write()) return 1;    // sends command to write command byte
    if (kbd_write_cmd(cmd)) return 1;      // writes command
    return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
    hook_timer = TIMER0_IRQ;
    *bit_no = hook_timer;
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

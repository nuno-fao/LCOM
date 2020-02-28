#include <lcom/lcf.h>
#include <stdint.h>
#include "utils.h"


extern uint8_t code;
extern int hook_kbd;
extern int mouse_hook;
extern int hook_timer;
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

int enable_kbd(){
    uint8_t cmd = 0;
    if (kbd_send_cmd_to_read()) return 1;    // sends command to read command byte
    if (kbd_read_cmd(&cmd)) return 1;     // reads command
    cmd |= BIT(0);
    if (kbd_send_cmd_to_write()) return 1;    // sends command to write command byte
    if (kbd_write_cmd(OUT_BUF, cmd)) return 1;      // writes command
    return 0;
}

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

int (mouse_subscribe_int)(uint16_t *bit_no) {
    mouse_hook=MOUSE_IRQ;
    *bit_no = BIT(mouse_hook);
    if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook)) return 1;
    return 0;
}

int (mouse_unsubscribe_int)() {
	if (sys_irqrmpolicy(&mouse_hook)) return 1;
    return 0;
}

void (mouse_ih)(){
    uint8_t status;
    new_sys_inb(STAT_REG, &status);
    if ((status & PAR_ERR) || (status & TO_ERR) || !(status & AUX_ERR)) {
		return;
	}
    if (status & OBF) {
        if (new_sys_inb(OUT_BUF, &code)) {
            code=0;
            return;
        }
	}
}

void parse_mouse_packet(struct packet *pp){
    if(pp->bytes[0]&MOUSE_X_B){
        pp->delta_x=((-(pp->bytes[1]))^0xFF) + 1;
    }
    else{
        pp->delta_x=pp->bytes[1];
    }

    if(pp->bytes[0]&MOUSE_Y_B){
        pp->delta_y=((-(pp->bytes[2]))^0xFF) + 1;
    }
    else{
        pp->delta_y=pp->bytes[2];
    }
    pp->x_ov=(pp->bytes[0] & MOUSE_XOVF);

    pp->y_ov=(pp->bytes[0] & MOUSE_YOVF);

    pp->lb=(pp->bytes[0] & MOUSE_LB);

    pp->rb=(pp->bytes[0] & MOUSE_RB);

    pp->mb=(pp->bytes[0] & MOUSE_MB);

}

int mouse_write_cmd(uint8_t cmd){
    uint8_t res;

    kbd_write_cmd(KBC_CMD_REG, MOUSE_W_CMD);
    kbd_write_cmd(OUT_BUF, cmd);
    tickdelay(micros_to_ticks(DELAY_US));
    kbd_read_cmd(&res);

    if(res != ACK){
        kbd_write_cmd(KBC_CMD_REG, MOUSE_W_CMD);
        kbd_write_cmd(OUT_BUF, cmd);
        tickdelay(micros_to_ticks(DELAY_US));
        kbd_read_cmd(&res);

        if(res == ERROR){
            return 1;
        }
    }

    return 0;
}

void gest(struct packet *pp, uint8_t x_len, uint8_t tolerance){
    extern bool done;
    enum event mouse = get_new_event(pp);
    static enum state state = INITIAL;
    static int16_t total_x, total_y;

    switch (state){
        case INITIAL:
            total_x = 0;
            total_y = 0;

            if(mouse == PRESSED_LB){
                state = DRAW_0;
            }
            break;

        case DRAW_0:
            if(mouse == PRESSED_RB || mouse == ANOTHER){
                state = INITIAL;
            } else if (mouse == RELEASED_LB){
                if(total_x >= x_len && (total_y/total_x) > 1){
                    state = BETWEEN;
                } else {
                    state = INITIAL;
                }
            } else if (mouse == MOVING) {
                if ((pp->delta_x > 0 && pp->delta_y > 0) || (abs(pp->delta_x) < tolerance && abs(pp->delta_y) < tolerance)){
                    total_x += pp->delta_x;
                    total_y += pp->delta_y;
                } else {
                    state = INITIAL;
                }
            }
            break;

        case BETWEEN:
            total_x = 0;
            total_y = 0;

            if (mouse == PRESSED_LB || mouse == ANOTHER){
                state = INITIAL;
            } else if (mouse == PRESSED_RB){
                state = DRAW_1;
            }
            break;

        case DRAW_1:
            if(mouse == PRESSED_LB || mouse == ANOTHER){
                state = INITIAL;
            } else if (mouse == RELEASED_RB){
                if(total_x >= x_len && abs(total_y/total_x) > 1){
                    state = INITIAL;
                    done = true;
                } else {
                    state = INITIAL;
                }
            } else if (mouse == MOVING) {
                if ((pp->delta_x > 0 && pp->delta_y < 0) || (abs(pp->delta_x) < tolerance && abs(pp->delta_y) < tolerance)){
                    total_x += pp->delta_x;
                    total_y += pp->delta_y;
                } else {
                    state = INITIAL;
                }
            }
            break;
    }
}

enum event get_new_event(struct packet *pp){
    static bool lb_was_pressed=false, rb_was_pressed=false;
    if(pp->lb && !lb_was_pressed){
        lb_was_pressed=true;
        return PRESSED_LB;
    }
    else if(!pp->lb && lb_was_pressed){
        lb_was_pressed=false;
        return RELEASED_LB;
    }
    else if(pp->rb && !rb_was_pressed){
        rb_was_pressed=true;
        return PRESSED_RB;
    }
    else if(!pp->rb && rb_was_pressed){
        rb_was_pressed=false;
        return RELEASED_RB;
    }
    
    else if((pp->delta_x!=0) || (pp->delta_y!=0)){
        return MOVING;
    }
    else {
        return ANOTHER;
    }
    
}

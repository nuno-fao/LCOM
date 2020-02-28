#include <lcom/lcf.h>
#include <stdint.h>
#include "utils.h"
#include "mouse.h"
#include "kbd.h"

extern int16_t deslocation;
extern uint8_t code;
extern int mouse_hook;

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

enum event get_new_event(struct packet *pp){
    static bool lb_was_pressed=false;
    if(pp->lb && !lb_was_pressed){
        lb_was_pressed=true;
        return PRESSED_LB;
    }
    else if(!pp->lb && lb_was_pressed){
        lb_was_pressed=false;
        return RELEASED_LB;
    }
    else if(pp->rb){
        return PRESSED_RB;
    }
    else if(pp->mb){
        return PRESSED_MB;
    }
    else if(pp->delta_x==0){
        return STANDING;
    }
    else if (pp->delta_x>0 && lb_was_pressed){
        deslocation=pp->delta_x;
        return MOVING_TO_RIGHT;
    }
    else if (lb_was_pressed){
        deslocation=pp->delta_x;
        return MOVING_TO_LEFT;
    }
    else{
        return STANDING;
    }
}

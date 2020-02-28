// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

#include "utils.h"

uint8_t code;
int hook_kbd;
int mouse_hook;
int hook_timer;
int time_elapsed;
char *video_mem;
unsigned h_res;
unsigned v_res;
unsigned bits_pp;
unsigned bytes_pp;
vbe_mode_info_t info;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
    if(vg_init(mode) == NULL){
        printf("Couldn't change mode!\n");
        return 1;
    }

    //delay
    sleep(delay);

    //reset text mode
    vg_exit();

    return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    if(vg_init(mode) == NULL){
        printf("Couldn't change mode!\n");
        return 1;
    }

    if(vg_draw_rectangle(x, y, width, height, color) == 1){
        printf("Couldn't draw rectangle!\n");
        return 1;
    }

    uint8_t size = 1;
    bool two_B = false;
    code = 0;
    uint8_t code_bytes[2];
    int ipc_status;
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
                    }
                    break;
                default:
                    break;
            }
        }
    }

    vg_exit();

    if(unsubscribe_kbd(&bit_no)){
        printf("Couldn't unsubscribe keyboard!\n");
        return 1;
    }

    return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

    if(vg_init(mode) == NULL){
        printf("Couldn't change mode!\n");
        return 1;
    }



    uint8_t size = 1;
    bool two_B = false;
    code = 0;
    uint8_t code_bytes[2];
    int ipc_status;
    message msg;
    int y=0, x;
    uint32_t color;
    
    int xlen=(h_res/no_rectangles);
    int ylen=(v_res/no_rectangles);


    for (unsigned int i = 0; i < no_rectangles; i++) {

        x = 0;

        for (unsigned int i2 = 0; i2 < no_rectangles; i2++) {
            color = get_color(no_rectangles, i2, i, first, step);
            if(vg_draw_rectangle(x, y, xlen, ylen, color) == 1){
                printf("Couldn't draw rectangle!\n");
                return 1;
            }
            x += xlen;
        }
        y += ylen;
    }

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
                    }
                    break;
                default:
                    break;
            }
        }
    }

    vg_exit();

    if(unsubscribe_kbd(&bit_no)){
        printf("Couldn't unsubscribe keyboard!\n");
        return 1;
    }

    return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {

    if(vg_init(0x105) == NULL){
        printf("Couldn't change mode!\n");
        return 1;
    }

    enum xpm_image_type type = XPM_INDEXED;
    xpm_image_t img;
    xpm_load(xpm, type, &img);

    xpm_draw(img.bytes, img.width, img.height, x, y);

    uint8_t size = 1;
    bool two_B = false;
    code = 0;
    uint8_t code_bytes[2];
    int ipc_status;
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
                    }
                    break;
                default:
                    break;
            }
        }
    }

    vg_exit();

    if(unsubscribe_kbd(&bit_no)){
        printf("Couldn't unsubscribe keyboard!\n");
        return 1;
    }

    return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate) {
    uint8_t bit_no_timer;
    uint8_t bit_no_kbd;
    uint8_t size = 1;
    uint16_t yspeed=0, xspeed=0;
    bool two_B = false;
    code = 0;
    uint8_t code_bytes[2];
    int ipc_status;
    message msg;
    time_elapsed=0;
    enum xpm_image_type type = XPM_INDEXED;

    if(fr_rate>60){
        printf("Framerate must be lower than 60!\n");
        return 1;
    }
    if(vg_init(0x105) == NULL){
        printf("Couldn't change mode!\n");
        return 1;
    }

    if (timer_subscribe_int(&bit_no_timer)){
		printf("Could not subscribe timer!\n");
		return 1;
	}

    if(kbd_subscribe(&bit_no_kbd) == 1){
        printf("Couldn't subscribe keyboard!\n");
        return 1;
    };
    uint32_t irq_set = BIT(bit_no_kbd);

    if(xf==xi){
        yspeed=speed;
        if(yf<yi && yspeed>0){
            yspeed=-yspeed;
        }
    }
    else if(yf==yi){
        xspeed=speed;
        if(xf<xi && xspeed>0){
            xspeed=-xspeed;
        }

    }
    else{
        printf("The movement is neither horizontal nor vertical!\n");
        return 1;
    }

    xpm_image_t img;
    xpm_load(xpm, type, &img);
    xpm_draw(img.bytes, img.width, img.height, xi, yi);

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
                    }
                    else if(msg.m_notify.interrupts & bit_no_timer){
                        timer_int_handler();
                        if(xf!=xi || yi!=yf){
                            if(speed>0 && time_elapsed%(60/fr_rate)==0){
                                if(xspeed<0 || yspeed<0){
                                    if(xi+xspeed<xf || yi+yspeed<yf){
                                        xi=xf;
                                        yi=yf;
                                    }
                                    else{
                                        xi+=xspeed;
                                        yi+=yspeed;
                                    }
                                }
                                else if(xspeed>0 || yspeed>0){
                                    if(xi+xspeed>xf || yi+yspeed>yf){
                                        xi=xf;
                                        yi=yf;
                                    }
                                    else{
                                        xi+=xspeed;
                                        yi+=yspeed;
                                    }
                                }
                                clear_VRAM();
                                xpm_draw(img.bytes, img.width, img.height, xi, yi);
                            }
                            else if(speed<0 && time_elapsed%(60/fr_rate)%abs(speed)){
                                if(xspeed!=0){
                                    xi++;
                                }
                                else{
                                    yi++;
                                }
                                clear_VRAM();
                                xpm_draw(img.bytes, img.width, img.height, xi, yi);
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    vg_exit();

    if (timer_unsubscribe_int()){
		printf("Could not unsubscribe mouse!\n");
		return 1;
	}
	
	if (unsubscribe_kbd()){
		printf("Could not unsubscribe keyboard!\n");
		return 1;
	}
    return 0;
}

int(video_test_controller)() {
  vg_vbe_contr_info_t info;
  get_info_controller(&info);
  
  return 0;
}

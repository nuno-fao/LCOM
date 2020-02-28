// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

// Any header files included below this line should have been created by you

#include "utils.h"
#include "game.h"
#include "timer.h"
#include "mouse.h"
#include "kbd.h"
#include "video.h"

uint8_t code;
uint32_t irq_set;
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
bool first;
enum xpm_image_type type = XPM_8_8_8;
char *buffer;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

//static int print_usage() {
//  printf("Usage: <mode - hex>\n");
//
//  return 1;
//}

int(proj_main_loop)(int argc, char *argv[]) {
    uint8_t bit_no_timer = 0;
    uint8_t bit_no_kbd = 1;
    uint16_t bit_no_mouse = 12;
    unsigned option;
    first = true;

    srand(time(NULL));

    if(vg_init(0x118) == NULL){
        printf("Couldn't change mode!\n");
        return 1;
    }

    buffer=malloc(h_res*v_res*bytes_pp);

    if(kbd_subscribe(&bit_no_kbd) == 1){
        printf("Couldn't subscribe keyboard!\n");
        return 1;
    };

    if (timer_subscribe_int(&bit_no_timer)){
		printf("Could not subscribe timer!\n");
		return 1;
	}

    if (mouse_subscribe_int(&bit_no_mouse)){
    printf("Couldn't subscribe mouse!\n");
    return 1;
    }

    sys_irqdisable(&mouse_hook);
    if (mouse_write_cmd(MOUSE_ENA_CMD)){
        printf("Couldn't enable data reporting!\n");
        return 1;
    }
    sys_irqenable(&mouse_hook);

    do{
        option = main_menu();
        switch(option){
            case 1:
                play();
                break;
            case 2:
                credits();
                break;
            case 3:
                break;
            default:
                break;
        }

    }while(option!=3);


    vg_exit();
    free(buffer);

    sys_irqdisable(&mouse_hook);
    if (mouse_write_cmd(MOUSE_DIS_CMD)){
        printf("Couldn't disable data reporting!\n");
        return 1;
    }
    sys_irqenable(&mouse_hook);

    if (mouse_unsubscribe_int()){
        printf("Could not unsubscribe mouse \n");
        return 1;
    }

    if (timer_unsubscribe_int()){
		printf("Could not unsubscribe timer!\n");
		return 1;
	}

    if(unsubscribe_kbd()){
        printf("Couldn't unsubscribe keyboard!\n");
        return 1;
    }

    return 0;
}

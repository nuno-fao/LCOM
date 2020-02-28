// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "utils.h"

uint8_t code;
int hook_kbd;
int mouse_hook;
int hook_timer;
int time_elapsed;
bool done = false;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {
  uint16_t bit_no;
  struct packet pp;
  uint32_t counter = 0;
  int byte=0;
  code=0;
  int r, ipc_status;
  message msg;


  if (mouse_subscribe_int(&bit_no))	{
    printf("Could not subscribe mouse!\n");
    return 1;
	}

  sys_irqdisable(&mouse_hook);
  if (mouse_write_cmd(MOUSE_ENA_CMD)){
    printf("Couldn't enable data reporting!\n");
    return 1;
  }
  sys_irqenable(&mouse_hook);

  while (counter<cnt)
	{ /* ends when it has printed the number in ctn */
		/*Get a request message.*/
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
		{
			printf("ERROR: driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status))
		{ // received notification
			switch (_ENDPOINT_P(msg.m_source))
			{
			case HARDWARE: // hardware interrupt notification
				if (msg.m_notify.interrupts & bit_no)
				{
					mouse_ih();
					if ((byte == 0 && (BIT(3) & code)) || byte == 1){								   
						pp.bytes[byte] = code;
						byte++;			
					}
					else if (byte == 2){		
            pp.bytes[byte] = code;					
						counter++;
            byte = 0;	
						parse_mouse_packet(&pp); 
						mouse_print_packet(&pp);		
					}
				}
				break;
			default:
				break;
			}
		}
	}

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

	return 0;

}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
    struct packet p;
    uint8_t count = 0;
    uint8_t byte_no = 0;
    uint8_t byte;

    while(count < cnt){
        if(byte_no == 0){
            if (mouse_write_cmd(MOUSE_R_CMD)){
                printf("Couldn't send command to read packet!\n");
                return 1;
            }
        }

        if(kbd_read_cmd(&byte)){
            tickdelay(micros_to_ticks(DELAY_US));
            continue;
        }

        p.bytes[byte_no] = byte;

        if((byte_no == 0 && (byte & BIT(3))) || byte_no == 1){
            byte_no++;
            continue;
        } else {
            parse_mouse_packet(&p);
            mouse_print_packet(&p);
            count++;
            byte_no = 0;
            tickdelay(micros_to_ticks(period * 1000));
        }
    }

    if (mouse_write_cmd(MOUSE_STREAM_CMD)){
        printf("Couldn't send command to set stream mode!\n");
        return 1;
    }

    if (mouse_write_cmd(MOUSE_DIS_CMD)){
        printf("Couldn't disable data reporting!\n");
        return 1;
    }

    if(kbd_write_cmd(KBC_CMD_REG, W_CMD)){
        printf("Couldn't send command to write!\n");
        return 1;
    }
    if(kbd_write_cmd(OUT_BUF, minix_get_dflt_kbc_cmd_byte())){
        printf("Couldn't reset default keyboard configuration!\n");
        return 1;
    }

    return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
  uint8_t bit_no_timer;
  uint16_t bit_no_mouse;
  struct packet pp;
  int byte=0;
  code=0;
  int r, ipc_status;
  message msg;
  time_elapsed=0;

  if(idle_time<=0){
    printf("Idle time must be a positive integer!\n");
    return 1;
  }

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

  while ((time_elapsed/60) < idle_time){
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
		{
			printf("ERROR: driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status))
		{ // received notification
			switch (_ENDPOINT_P(msg.m_source))
			{
			case HARDWARE: // hardware interrupt notification
				if (msg.m_notify.interrupts & bit_no_mouse)
				{				 // mouse interrupt
          time_elapsed=0;
          mouse_ih();
					if ((byte == 0 && (BIT(3) & code)) || byte == 1){								   
						pp.bytes[byte] = code;
						byte++;			
					}
					else if (byte == 2){		
            pp.bytes[byte] = code;
            byte = 0;	
						parse_mouse_packet(&pp); 
						mouse_print_packet(&pp);		
					}
				}
				else if (msg.m_notify.interrupts & bit_no_timer){	
					timer_int_handler(); 
				}
				break;
			default:
				break;
			}
		}
		else
		{   // received a standard message, not a notification
			// no standard messages expected: do nothing
		}
	}
  sys_irqdisable(&mouse_hook);
    if (mouse_write_cmd(MOUSE_DIS_CMD)){
        printf("Couldn't disable data reporting!\n");
        return 1;
    }
  sys_irqenable(&mouse_hook);
  if (mouse_unsubscribe_int()){
		printf("Could not unsubscribe mouse!\n");
		return 1;
	}
	
	if (timer_unsubscribe_int()){
		printf("Could not unsubscribe timer!\n");
		return 1;
	}
  return 0;
}

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
    uint16_t bit_no;
    struct packet pp;
    int byte=0;
    code=0;
    int r, ipc_status;
    message msg;


    if (mouse_subscribe_int(&bit_no))	{
        printf("Could not subscribe mouse!\n");
        return 1;
    }

    sys_irqdisable(&mouse_hook);
    if (mouse_write_cmd(MOUSE_ENA_CMD)){
        printf("Couldn't enable data reporting!\n");
        return 1;
    }
    sys_irqenable(&mouse_hook);

    while (!done)
    {

        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
        {
            printf("ERROR: driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status))
        {
            switch (_ENDPOINT_P(msg.m_source))
            {
                case HARDWARE:
                    if (msg.m_notify.interrupts & bit_no)
                    {
                        mouse_ih();
                        if ((byte == 0 && (BIT(3) & code)) || byte == 1){
                            pp.bytes[byte] = code;
                            byte++;
                        }
                        else if (byte == 2){
                            pp.bytes[byte] = code;
                            byte = 0;
                            parse_mouse_packet(&pp);
                            gest(&pp, x_len, tolerance);
                            mouse_print_packet(&pp);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

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

    return 0;
}

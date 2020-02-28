#include <lcom/lcf.h>
#include <stdint.h>
#include "utils.h"

extern char *video_mem;
extern unsigned h_res;
extern unsigned v_res;
extern char *buffer;
extern unsigned bytes_pp;

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

void clear_buffer(){
    for (unsigned int i = 0; i < h_res * v_res*bytes_pp; i++) {
        *(buffer + i) = 0;
    }
}


unsigned bcd_to_decimal(uint32_t x) {
    return x - 6 * (x >> 4);
}

void copybuffer(){
    memcpy(video_mem,buffer,h_res*v_res*bytes_pp);
}

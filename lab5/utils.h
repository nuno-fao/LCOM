#pragma once

/** @defgroup utils utils
 * @{
 *
 * LCOM's utility functions
 */

#include <stdint.h>

#define BIT(n) (1 << (n))
#define STAT_REG 0x64
#define KBC_CMD_REG 0x64
#define OUT_BUF 0x60
#define KBD_IRQ 0x01
#define MOUSE_IRQ 12
#define OBF BIT(0)
#define IBF BIT(1)
#define ESC_BC 0x81
#define MAKE BIT(7)
#define PAR_ERR BIT(7)
#define TO_ERR BIT(6)
#define AUX_ERR BIT(5)
#define TWO_BYTE_CODE 0xE0
#define AUX BIT(5)
#define DELAY_US 20000
#define TIMER0_IRQ 0
#define R_CMD 0x20
#define W_CMD 0x60
#define MOUSE_W_CMD 0xD4
#define MOUSE_R_CMD 0xEB
#define MOUSE_STREAM_CMD 0xEA
#define MOUSE_DIS_CMD 0xF5
#define MOUSE_ENA_CMD 0xF4
#define MOUSE_LB BIT(0)
#define MOUSE_RB BIT(1)
#define MOUSE_MB BIT(2)
#define MOUSE_X_B BIT(4)
#define MOUSE_Y_B BIT(5)
#define MOUSE_XOVF BIT(6)
#define MOUSE_YOVF BIT(7)
#define ACK     0xFA
#define NACK    0xFE
#define ERROR   0xFC


/**
 * @brief Returns the LSB of a 2 byte integer
 *
 * @param val input 2 byte integer
 * @param lsb address of memory location to be updated with val's LSB
 * @return Return 0 upon success and non-zero otherwise
 */
int (util_get_LSB)(uint16_t val, uint8_t *lsb);

/**
 * @brief Returns the MSB of a 2 byte integer
 *
 * @param val input 2 byte integer
 * @param msb address of memory location to be updated with val's LSB
 * @return Return 0 upon success and non-zero otherwise
 */
int (util_get_MSB)(uint16_t val, uint8_t *msb);

/**
 * @brief Invokes sys_inb() system call but reads the value into a uint8_t variable.
 *
 * @param port the input port that is to be read
 * @param value address of 8-bit variable to be update with the value read
 * @return Return 0 upon success and non-zero otherwise
 */
int (util_sys_inb)(int port, uint8_t *value);

int new_sys_inb(port_t port, uint8_t *byte);

int (kbd_subscribe)(uint8_t *bit_no);
int (unsubscribe_kbd)();

void (kbc_ih)();

int enable_kbd();

int (timer_subscribe_int)(uint8_t *bit_no);

int (timer_unsubscribe_int)();

void (timer_int_handler)();

int kbd_send_cmd_to_read();

int kbd_read_cmd(uint8_t *cmd);

int kbd_send_cmd_to_write();

int kbd_write_cmd(port_t port, uint8_t cmd);

int mouse_subscribe_int(uint16_t *bit__no);

int mouse_unsubscribe_int();

void parse_mouse_packet(struct packet *pp);

int mouse_write_cmd(uint8_t cmd);

enum event{
  PRESSED_RB,
  PRESSED_LB,
  RELEASED_RB,
  RELEASED_LB,
  MOVING,
  ANOTHER
};

enum event get_new_event(struct packet *pp);

void *(vg_init)(uint16_t mode);

int draw_pixel(uint16_t x, uint16_t y, uint32_t color);

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

uint32_t get_color(uint8_t no_rectangles, uint8_t col, uint8_t row, uint32_t first, uint8_t step);

void xmp_draw_line(uint8_t *xpm, uint16_t w, uint16_t x, uint16_t y);

void xpm_draw(uint8_t *xpm, uint16_t w, uint16_t h, uint16_t x, uint16_t y);

void clear_VRAM();

int get_info_controller(vg_vbe_contr_info_t *info_holder);


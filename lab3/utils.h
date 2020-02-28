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
#define OBF BIT(0)
#define IBF BIT(1)
#define ESC_BC 0x81
#define MAKE BIT(7)
#define PAR_ERR BIT(7)
#define TO_ERR BIT(6)
#define TWO_BYTE_CODE 0xE0
#define AUX BIT(5)
#define DELAY_US 20000
#define TIMER0_IRQ 0
#define R_CMD 0x20
#define W_CMD 0x60

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

int kbd_write_cmd(uint8_t cmd);

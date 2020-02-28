#pragma once

/** @defgroup utils utils
 * @{
 *
 * LCOM's utility functions
 */

#include <stdint.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "xpms/MENU.xpm"
#include "xpms/ARROW.xpm"
#include "xpms/CREDITS.xpm"
#include "xpms/GBEGIN.xpm"
#include "xpms/GEND.xpm"
#include "xpms/NEXTLEVEL.xpm"
#include "xpms/BLOCK.xpm"
#include "xpms/PLAYER.xpm"
#include "xpms/BPLAYER.xpm"
#include "xpms/GPAUSE.xpm"
#include "xpms/BALL.xpm"
#include "xpms/TBONUS.xpm"
#include "xpms/PBONUS.xpm"
#include "xpms/0.xpm"
#include "xpms/1.xpm"
#include "xpms/2.xpm"
#include "xpms/3.xpm"
#include "xpms/4.xpm"
#include "xpms/5.xpm"
#include "xpms/6.xpm"
#include "xpms/7.xpm"
#include "xpms/8.xpm"
#include "xpms/9.xpm"
#include "xpms/SLASH.xpm"
#include "xpms/DOUBLE.xpm"

#define BIT(n) (1 << (n))
#define STAT_REG 0x64
#define KBC_CMD_REG 0x64
#define OUT_BUF 0x60
#define KBD_IRQ 0x01
#define MOUSE_IRQ 12
#define OBF BIT(0)
#define IBF BIT(1)
#define ESC_BC 0x81
#define ENTER_BC 0x9C
#define MAKE BIT(7)
#define PAR_ERR BIT(7)
#define TO_ERR BIT(6)
#define AUX_ERR BIT(5)
#define TWO_BYTE_CODE 0xE0
#define AUX BIT(5)
#define DELAY_US 20000
#define TIMER0_IRQ 0x00
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
#define UPBREAK 0xC8
#define DOWNBREAK 0xD0
#define MILLENIUM 2000
#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71
#define RTC_SECONDS 0
#define RTC_MINUTES 2
#define RTC_HOURS 	4
#define RTC_DAYOFWEEK 6
#define RTC_DAYOFMONTH 7
#define RTC_MONTH 8
#define RTC_YEAR 9

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
int new_sys_inb(port_t port, uint8_t *byte);

/**
 * @brief Fills the buffer's memory block with 0
 * 
 * @return Returns nothing
 */
void clear_buffer();

/**
 * @brief Copies the buffer's memory block to the VRAM
 * 
 * @return Returns nothing
 */
void copybuffer();

/**
 * @brief Converts a BCD into decimal
 * 
 * @param x number in BCD
 * 
 * @return Returns the unsigned equivalent to the BCD parameter
 */
unsigned bcd_to_decimal(uint32_t x);

/** }@ */

#pragma once

/** @defgroup keyboard keyboard
 * @{
 *
 * Keyboard utility functions
 */

/**
 * @brief Subscribes keyboard interrupts
 *
 * @param bit_no address of 8-bit variable that has the IRQ which the keyboard raises
 * @return Return 0 upon success and non-zero otherwise
 */
int (kbd_subscribe)(uint8_t *bit_no);

/**
 * @brief Unsubscribes keyboard interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int (unsubscribe_kbd)();

/**
 * @brief Reads the status from the status register, to check if there isn't any communications error, and reads the scancode byte from the output buffer into a global variable.
 *
 * @return Return 0 upon success and non-zero otherwise
 */
void (kbc_ih)();

/**
 * @brief Reads the KBC "command byte"
 *
 * @param cmd address of 8-bit variable that will hold the KBC "command byte"
 * @return Return 0 upon success and non-zero otherwise
 */
int kbd_read_cmd(uint8_t *cmd);

/**
 * @brief Writes a command to a port, both specified in the arguments
 *
 * @param port target port
 * @param cmd commad to write
 * @return Return 0 upon success and non-zero otherwise
 */
int kbd_write_cmd(port_t port, uint8_t cmd);

/** }@ */

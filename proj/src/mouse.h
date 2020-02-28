#pragma once


enum event{
  PRESSED_RB, /**< right-button was pressed */
  PRESSED_LB, /**< left-button was pressed */
  RELEASED_LB, /**< left-button was released */
  PRESSED_MB, /**< middle-button was pressed */
  MOVING_TO_LEFT, /**< mouse was moved to the left */
  MOVING_TO_RIGHT, /**< mouse was moved to the right */
  STANDING /**< any other event */
};

/**
 * @brief Subscribes mouse interrupts
 *
 * @param bit_no address of 8-bit variable that has the IRQ which the mouse raises
 * @return Return 0 upon success and non-zero otherwise
 */
int mouse_subscribe_int(uint16_t *bit__no);

/**
 * @brief Unsubscribes mouse interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int mouse_unsubscribe_int();

/**
 * @brief Reads the status from the status register, to check if there isn't any communications error, and reads the scancode byte from the output buffer into a global variable (reads only one byte).
 *
 * @return Returns nothing
 */
void (mouse_ih)();

/**
 * @brief With the 3 bytes in the pp param this function will initialise the rest of the struct members accordingly (buttons, x and y positiong, x and y overflow)
 *
 * @param pp address of struct packet that holds the 3 bytes read from previous mouse interrupts
 * 
 * @return Returns nothing
 */
void parse_mouse_packet(struct packet *pp);

/**
 * @brief Writes a command to the KBC "command byte"
 *
 * @param cmd commad to write
 * @return Return 0 upon success and non-zero otherwise
 */
int mouse_write_cmd(uint8_t cmd);

/**
 * @brief With the parsed pp param the function will detect what mouse event went on and return an enum event that represents it
 *
 * @param pp address of struct packet already parsed
 * 
 * @return Returns an enum that represents the mouse event
 */
enum event get_new_event(struct packet *pp);

/** }@ */

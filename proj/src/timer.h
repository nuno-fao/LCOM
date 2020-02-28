#pragma once

/** @defgroup timer timer
 * @{
 *
 * Timer and RTC utility functions
 */


struct Date{
    unsigned year; /**< RTC year */
    unsigned month; /**< RTC month */
    unsigned day; /**< RTC day */
    unsigned hour; /**< RTC hour */
    unsigned minute; /**< RTC minute */
    unsigned second; /**< RTC second */
};

/**
 * @brief Subscribes timer interrupts
 *
 * @param bit_no address of 8-bit variable that has the IRQ which the timer raises
 * @return Return 0 upon success and non-zero otherwise
 */
int (timer_subscribe_int)(uint8_t *bit_no);

/**
 * @brief Unsubscribes timer interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int (timer_unsubscribe_int)();

/**
 * @brief Increments a global variable upon each call
 *
 * @return Returns nothing
 */
void (timer_int_handler)();

/**
 * @brief Reads the real time clock
 * 
 * @param real_date address to a Date struct that will hold the real time at the end of the function
 * 
 * @return Returns the unsigned equivalent to the BCD parameter
 */
void rtc_time(struct Date *real_date);

/** }@ */

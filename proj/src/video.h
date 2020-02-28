#pragma once

/** @defgroup video video
 * @{
 *
 * Video Card utility functions
 */

/**
 * @brief Switches the video adapter to the graphics mode specified in its argument, using the VBE interface
 *
 * @param mode holds the mode to which the video adapter will change to
 * 
 * @return Returns nothing
 */
void *(vg_init)(uint16_t mode);

/**
 * @brief Draws a pixel at a specific location and with a specific color
 *
 * @param x horizontal location to draw
 * @param y vertical location to draw
 * @param color color to draw
 * 
 * @return Return 0 upon success and non-zero if x and y arguments are bigger than the window boundaries
 */
int draw_pixel(uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Draws a line of pixels at a specific location and with a specific color
 *
 * @param x horizontal location to draw
 * @param y vertical location to draw
 * @param len length of pixel line
 * @param color color to draw
 * 
 * @return Return 0 upon success and non-zero if x and y arguments are bigger than the window boundaries
 */
int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

/**
 * @brief Draws a rectangle at a specific location and with a specific color
 *
 * @param x horizontal location to start drawing
 * @param y vertical location to start drawing
 * @param width length of pixel line
 * @param height heigth of pixel line
 * @param color color to draw
 * 
 * @return Return 0 upon success and non-zero if x and y arguments are bigger than the window boundaries
 */
int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

/**
 * @brief Draws a rectangle at a specific location and with a specific color
 *
 * @param xpm address to the xpm that will be drawn
 * @param w width of the xpm
 * @param x horizontal location to start drawing
 * @param y vertical location to start drawing
 * 
 * @return Returns nothing
 */
void xmp_draw_line(uint8_t *xpm, uint16_t w, uint16_t x, uint16_t y);

/**
 * @brief Draws a rectangle at a specific location and with a specific color
 *
 * @param xpm address to the xpm that will be drawn
 * @param w width of the xpm
 * @param h height of the xpm
 * @param x horizontal location to start drawing
 * @param y vertical location to start drawing
 * 
 * @return Returns nothing
 */
void xpm_draw(uint8_t *xpm, uint16_t w, uint16_t h, uint16_t x, uint16_t y);

/**
 * @brief Draws specific line of the xpm
 *
 * @param xpm address to the xpm that will be partially drawn
 * @param w width of the xpm
 * @param wsubs width to draw
 * @param h height of the xpm
 * @param x horizontal location to start drawing
 * @param y vertical location to start drawing
 * 
 * @return Returns nothing
 */
void xmp_draw_subs_line(uint8_t *xpm, uint16_t w, uint16_t wsubs, uint16_t x, uint16_t y);

/**
 * @brief Draws specific portion of an xpm at the same address of video. XPM must have the same dimensions as the video mode
 *
 * @param xpm address to the xpm that will be partially drawn
 * @param w width of the xpm
 * @param wsubs width to draw
 * @param h height of the xpm
 * @param hsubs height to draw
 * @param x horizontal location to start drawing
 * @param y vertical location to start drawing
 * 
 * @return Returns nothing
 */
void xpm_subs(uint8_t *xpm, uint16_t w, uint16_t h, uint16_t wsubs, uint16_t hsubs, uint16_t x, uint16_t y);

/** }@ */

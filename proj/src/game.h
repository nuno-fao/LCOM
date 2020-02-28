#pragma once

/** @defgroup game game
 * @{
 *
 * Game functions
 */

struct Date; // only here because this file is compiled before the file where struct Date is declared

/**
 * @brief Main function to draw and handle everything that is related to the main menu
 *
 * @return Returns an unsigned that represents what to do next (1-play, 2-credits, 3-exit)
 */
unsigned main_menu();

/**
 * @brief Main function that handles everything related to playing the actual game. While this function is running the game is being played
 *
 * @return Returns nothing
 */
void play();

/**
 * @brief Draws an xpm that shows the credits
 *
 * @return Returns nothing
 */
void credits();

/**
 * @brief Draws the player at his default position (384,688)
 * 
 * @param xpm address to xpm that represents the player
 * 
 * @return Returns nothing
 */
void draw_player(uint8_t *xpm);

/**
 * @brief Draws the ball at its default position (512,624) and updates its speed according to the level
 * 
 * @param ball address to xpm that represents the ball
 * @param level level the players is currently on
 * 
 * @return Returns nothing
 */
void draw_ball(uint8_t *ball, unsigned level);

/**
 * @brief Draws the grid of undestroyed blocks
 * 
 * @return Returns nothing
 */
void draw_grid();

/**
 * @brief Given a number n this function draws an xpm equivalent to the number at y=0 and x=xi
 * 
 * @param xi starting horizontal position
 * @param n number to draw
 * 
 * @return Returns nothing
 */
void draw_number(uint16_t xi, unsigned n);

/**
 * @brief Draws a new level with the player and ball at thei default position, but first a confirmation of finishing the last level. Also draws the score and remaining time
 * 
 * @param level level to draw
 * @param player address to the player's xpm
 * @param ball address to the ball's xpm
 * @param time_count address to the remaining time
 * @param score current score
 * 
 * @return Returns nothing
 */
void next_level(unsigned *level, uint8_t *player, uint8_t *ball, unsigned *time_count, unsigned score);

/**
 * @brief Draws an xpm that confirms that the last level was finished and ask for the player to press the mouse's right button so that he can proceed: If it the first level it instead draws an xpm to explain how the game works
 * 
 * @param level level that will be played
 * 
 * @return Returns nothing
 */
void next_level_card(unsigned level);

/**
 * @brief Checks if the ball would collide with anything on the next step
 * 
 * @param level the level that is being played
 * 
 * @return Returns 0 there isn't any collision, 1 if a block needs to be destroyed and 2 if the players loses due to the ball passing the y limit
 */
int handle_ball_collision(unsigned level);


/**
 * @brief Draws a date read from the RTC in a specific location
 * 
 * @param real_date address that holds the date to draw
 * @param x horizontal position to start to draw
 * @param y vertical position to start to draw
 * 
 * @return Returns the unsigned equivalent to the BCD parameter
 */
void draw_real_date(struct Date *real_date, uint16_t x, uint16_t y);

/**
 * @brief Function the is called when the game is paused. Draws the RTC and updates it and also asks if the players wants to continue or quit
 * 
 * @return Returns true if the player wants to return to the main menu and false if he intends to continue
 */
bool pause_game();

/** }@ */

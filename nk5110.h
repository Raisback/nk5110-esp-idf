#ifndef NK5110_H
#define NK5110_H

#include <stdint.h>
#include "driver/spi_master.h"

// --- Hardware Configuration ---
// These are the specific GPIO pins the LCD module is connected to.
#define PIN_NUM_MISO 13 // Not used for this unidirectional display
#define PIN_NUM_MOSI 11 // DIN (Data In)
#define PIN_NUM_CLK  12 // CLK (Clock)
#define PIN_NUM_CS    4 // CS (Chip Select)
#define PIN_NUM_DC   10 // D/C (Data/Command)
#define PIN_NUM_RST  14 // RST (Reset)

// --- LCD Dimensions and Frame Buffer Size ---
#define LCD_WIDTH   84
#define LCD_HEIGHT  48
#define LCD_FRAME_SIZE (LCD_WIDTH * (LCD_HEIGHT / 8)) // 504 bytes (84 * 6)

// --- Font Definitions ---
#define FONT_START_CHAR 32 
#define FONT_CHAR_WIDTH 5
#define FONT_CHAR_HEIGHT 8

// --- Global Data Declarations (Defined in nokia5110.c) ---
extern spi_device_handle_t spi;
extern uint8_t frame_buffer[LCD_FRAME_SIZE]; 

// --- Public Function Prototypes ---

/**
 * @brief Initializes the GPIO pins, SPI bus, and the LCD module.
 */
void nk5110_init();

/**
 * @brief Sends the content of the frame buffer to the LCD.
 */
void nk5110_update_display();

/**
 * @brief Fills the frame buffer with a specific byte pattern (e.g., 0x00 for clear, 0xFF for solid black).
 * @param pattern The byte pattern to fill the buffer with.
 */
void nk5110_fill_buffer(uint8_t pattern);

/**
 * @brief Sets or clears a single pixel in the frame buffer.
 * @param x X coordinate (0 to LCD_WIDTH-1).
 * @param y Y coordinate (0 to LCD_HEIGHT-1).
 * @param set 1 to turn pixel ON (black), 0 to turn pixel OFF (white).
 */
void nk5110_set_pixel(uint8_t x, uint8_t y, uint8_t set);

/**
 * @brief Draws a line between two points using Bresenham's algorithm.
 */
void nk5110_draw_line(int x1, int y1, int x2, int y2, uint8_t set);

/**
 * @brief Draws the outline of a rectangular box.
 */
void nk5110_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t set);

/**
 * @brief Draws a solid filled rectangular box.
 */
void nk5110_draw_frect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t set);

/**
 * @brief Draws the outline of a rectangle with rounded corners.
 */
void nk5110_draw_rrect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t set); 

/**
 * @brief Draws a solid filled rectangle with rounded corners.
 */
void nk5110_draw_rfrect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t set); 

/**
 * @brief Draws a single character from the font table.
 */
void nk5110_draw_char(uint8_t x, uint8_t y, char c, uint8_t set);

/**
 * @brief Draws a null-terminated string.
 */
void nk5110_draw_str(uint8_t x, uint8_t y, const char *str, uint8_t set);

/**
 * @brief Draws a monochromatic bitmap image.
 * @param bitmap Pointer to the raw bitmap data.
 */
void nk5110_draw_bitmap(uint8_t x_start, uint8_t y_start, uint8_t w, uint8_t h, const uint8_t *bitmap, uint8_t set);

#endif // NK5110_H


#include "nk5110.h"
#include <string.h> // Keep: Used for memset
#include "freertos/FreeRTOS.h" // Keep: Used for vTaskDelay
#include "freertos/task.h"     // Keep: Used for vTaskDelay
#include "esp_log.h"         // Keep: Used for ESP_LOGI
#include "driver/gpio.h"     // Keep: Used for gpio_set_direction/level
#include <stdlib.h>          // Keep: Used for abs (in nk5110_draw_line)

// Removed: <stdio.h> (Not strictly needed if only using ESP_LOG macros)
// Removed: "esp_timer.h" (Not used in this file)

// --- Local Constants and Definitions ---
static const char *TAG = "NK5110_DRIVER";
#define BYTES_PER_FRAME 60 // Used internally for bitmap drawing safety check

// --- Global Data Definitions ---
spi_device_handle_t spi;
uint8_t frame_buffer[LCD_FRAME_SIZE]; // Global frame buffer

// Font Data Definition
#define FONT_TABLE_SIZE 95 
const uint8_t font_table[][5] = {
    // 32 ' '
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // 33 '!' -> 47 '/'
    {0x00, 0x00, 0x5F, 0x00, 0x00}, {0x00, 0x07, 0x00, 0x07, 0x00}, {0x14, 0x7F, 0x14, 0x7F, 0x14}, {0x24, 0x2A, 0x7F, 0x2A, 0x12}, {0x23, 0x13, 0x08, 0x64, 0x62}, {0x36, 0x49, 0x56, 0x20, 0x50}, {0x00, 0x05, 0x03, 0x00, 0x00}, {0x00, 0x1C, 0x22, 0x41, 0x00}, {0x00, 0x41, 0x22, 0x1C, 0x00}, {0x08, 0x2A, 0x1C, 0x2A, 0x08}, {0x08, 0x08, 0x3E, 0x08, 0x08}, {0x00, 0x50, 0x30, 0x00, 0x00}, {0x08, 0x08, 0x08, 0x08, 0x08}, {0x00, 0x60, 0x60, 0x00, 0x00}, {0x20, 0x10, 0x08, 0x04, 0x02},
    // 48 '0' -> 57 '9'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00}, {0x62, 0x51, 0x49, 0x46, 0x40}, {0x22, 0x41, 0x49, 0x49, 0x36}, {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x2F, 0x49, 0x49, 0x49, 0x31}, {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03}, {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x49, 0x3E},
    // 58 ':' -> 64 '@'
    {0x00, 0x36, 0x36, 0x00, 0x00}, {0x00, 0x56, 0x36, 0x00, 0x00}, {0x08, 0x14, 0x22, 0x41, 0x00}, {0x14, 0x14, 0x14, 0x14, 0x14}, {0x00, 0x41, 0x22, 0x14, 0x08}, {0x02, 0x01, 0x51, 0x09, 0x06}, {0x3E, 0x41, 0x59, 0x49, 0x7E},
    // 65 'A' -> 90 'Z' (Uppercase Alphabet)
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36}, {0x3E, 0x41, 0x41, 0x41, 0x22}, {0x7F, 0x41, 0x41, 0x41, 0x3E}, {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x01, 0x01}, {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F}, {0x00, 0x41, 0x7F, 0x41, 0x00}, {0x30, 0x40, 0x40, 0x40, 0x7F}, {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40}, {0x7F, 0x02, 0x04, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F}, {0x3E, 0x41, 0x41, 0x41, 0x3E}, {0x7F, 0x09, 0x09, 0x09, 0x06}, {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46}, {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7F, 0x01, 0x01}, {0x3F, 0x40, 0x40, 0x40, 0x3F}, {0x1F, 0x20, 0x40, 0x20, 0x1F}, {0x3F, 0x40, 0x30, 0x40, 0x3F}, 
    {0x41, 0x22, 0x1C, 0x22, 0x41}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, {0x41, 0x61, 0x51, 0x49, 0x46},
    // 91 '[' -> 96 '`'
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [ 
    {0x02, 0x04, 0x08, 0x10, 0x20}, // 
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    // 97 'a' -> 122 'z' (Lowercase Alphabet)
    {0x20, 0x54, 0x54, 0x54, 0x78},  // a
    {0x7F, 0x44, 0x44, 0x44, 0x38},  // b
    {0x38, 0x44, 0x44, 0x44, 0x20},  // c
    {0x38, 0x44, 0x44, 0x44, 0x7F},  // d
    {0x38, 0x54, 0x54, 0x54, 0x18},  // e
    {0x08, 0x7E, 0x09, 0x01, 0x02},  // f
    {0x08, 0x54, 0x54, 0x54, 0x3c},  // g
    {0x7F, 0x04, 0x04, 0x04, 0x78},  // h
    {0x00, 0x44, 0x7D, 0x40, 0x00},  // i
    {0x20, 0x40, 0x44, 0x3D, 0x00},  // j
    {0x7F, 0x10, 0x28, 0x44, 0x00},  // k
    {0x00, 0x01, 0x7F, 0x40, 0x00},  // l 
    {0x7C, 0x04, 0x18, 0x04, 0x78},  // m
    {0x7C, 0x04, 0x04, 0x04, 0x78},  // n
    {0x38, 0x44, 0x44, 0x44, 0x38},  // o
    {0x7C, 0x14, 0x14, 0x14, 0x08},  // p 
    {0x08, 0x14, 0x14, 0x14, 0x7C},  // q 
    {0x7C, 0x04, 0x04, 0x04, 0x08},  // r
    {0x48, 0x54, 0x54, 0x54, 0x20},  // s
    {0x04, 0x3E, 0x44, 0x44, 0x24},  // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C},  // u
    {0x1C, 0x20, 0x40, 0x20, 0x1F},  // v
    {0x3F, 0x40, 0x30, 0x40, 0x3F},  // w
    {0x44, 0x28, 0x10, 0x28, 0x44},  // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C},  // y
    {0x64, 0x54, 0x54, 0x54, 0x4C},  // z
    
    // 123 '{' -> 126 '~'
    {0x00, 0x36, 0x49, 0x41, 0x00}, // { 
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // | 
    {0x00, 0x41, 0x49, 0x36, 0x00}, // } 
    {0x00, 0x40, 0x20, 0x40, 0x20}, // ~ 
};


// --- Internal Function Prototypes ---
static void gpio_init();
static void spi_init();
static void lcd_command(uint8_t cmd);
static void lcd_data(const uint8_t *data, size_t len);
static void lcd_reset();
static void lcd_init();
static void draw_fast_hLine(int16_t x, int16_t y, int16_t w, uint8_t set);
static void draw_round_circle_quarter(int16_t x0, int16_t y0, int16_t r, uint8_t corner, uint8_t set);
static void fill_round_circle_quarter(int16_t x0, int16_t y0, int16_t r, uint8_t corner, uint8_t set);


// --- Library Public Entry Point ---

void nk5110_init() {
    gpio_init();
    spi_init();
    lcd_init();
    nk5110_fill_buffer(0x00);
    nk5110_update_display();
    ESP_LOGI(TAG, "Nokia 5110 Display Driver (NK5110) Initialized on SPI2.");
}


// --- Initialization, SPI, and LCD Control Implementations ---

static void gpio_init() {
    // LCD Control Pins (D/C, RST, CS)
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    
    gpio_set_level(PIN_NUM_RST, 0); 
    gpio_set_level(PIN_NUM_CS, 1);
}

static void spi_init() {
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO, 
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK, 
        .quadwp_io_num = -1, 
        .quadhd_io_num = -1,
        .max_transfer_sz = 512, 
    };

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000, 
        .mode = 0,                         
        .spics_io_num = -1,                
        .queue_size = 7, 
        .pre_cb = NULL,
        .command_bits = 0, 
        .address_bits = 0,
    };

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

static void lcd_command(uint8_t cmd) {
    spi_transaction_t t = {.length = 8, .tx_buffer = &cmd, .user = (void*)0};
    
    gpio_set_level(PIN_NUM_CS, 0); 
    gpio_set_level(PIN_NUM_DC, 0); 
    
    spi_device_polling_transmit(spi, &t);
    
    gpio_set_level(PIN_NUM_CS, 1);
}

static void lcd_data(const uint8_t *data, size_t len) {
    if (len == 0) return;
    spi_transaction_t t = {.length = len * 8, .tx_buffer = data, .user = (void*)1};
    
    gpio_set_level(PIN_NUM_CS, 0); 
    gpio_set_level(PIN_NUM_DC, 1); 
    
    spi_device_polling_transmit(spi, &t);
    
    gpio_set_level(PIN_NUM_CS, 1);
}

static void lcd_reset() {
    gpio_set_level(PIN_NUM_RST, 0); 
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(PIN_NUM_RST, 1); 
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void lcd_init() {
    lcd_reset();
    
    // PCD8544 initialization sequence
    lcd_command(0x21);           // Function set (Extended Instruction set)
    lcd_command(0x13);           // Bias System (1:48)
    lcd_command(0x80 | 0x3F);    // Vop (Contrast)
    lcd_command(0x04);           // Temperature Coefficient
    lcd_command(0x20);           // Function set (Basic Instruction set)
    lcd_command(0x0C);           // Display control (Normal Mode: 0x0C)
}

// --- Public Primitives Implementation ---

void nk5110_update_display() {
    lcd_command(0x80 | 0); // Set X address to 0
    lcd_command(0x40 | 0); // Set Y address (bank) to 0
    lcd_data(frame_buffer, LCD_FRAME_SIZE);
}

void nk5110_fill_buffer(uint8_t pattern) {
    memset(frame_buffer, pattern, LCD_FRAME_SIZE);
}

void nk5110_set_pixel(uint8_t x, uint8_t y, uint8_t set) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    uint16_t index = x + (y / 8) * LCD_WIDTH;
    uint8_t mask = 1 << (y % 8);
    
    if (set) {
        frame_buffer[index] |= mask;  // Set bit (pixel ON / Black)
    } else {
        frame_buffer[index] &= ~mask; // Clear bit (pixel OFF / White)
    }
}

// --- Internal Helper Primitives ---

/**
 * @brief Internal: Draws a fast horizontal line. Used by fill functions.
 */
static void draw_fast_hLine(int16_t x, int16_t y, int16_t w, uint8_t set) {
    if (y < 0 || y >= LCD_HEIGHT) return;
    for (int16_t i = 0; i < w; i++) {
        nk5110_set_pixel(x + i, y, set);
    }
}

/**
 * @brief Internal: Draws a quarter circle outline using the midpoint circle algorithm.
 */
static void draw_round_circle_quarter(int16_t x0, int16_t y0, int16_t r, uint8_t corner, uint8_t set) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (corner & 0x1) { nk5110_set_pixel(x0 + x, y0 - y, set); nk5110_set_pixel(x0 + y, y0 - x, set); } // Top Right
        if (corner & 0x2) { nk5110_set_pixel(x0 - y, y0 - x, set); nk5110_set_pixel(x0 - x, y0 - y, set); } // Top Left
        if (corner & 0x4) { nk5110_set_pixel(x0 - x, y0 + y, set); nk5110_set_pixel(x0 - y, y0 + x, set); } // Bottom Left
        if (corner & 0x8) { nk5110_set_pixel(x0 + y, y0 + x, set); nk5110_set_pixel(x0 + x, y0 + y, set); } // Bottom Right
    }
}

/**
 * @brief Internal: Fills a quarter circle using fast horizontal line drawing.
 */
static void fill_round_circle_quarter(int16_t x0, int16_t y0, int16_t r, uint8_t corner, uint8_t set) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x <= y) {
        // Corner flags (1-4) are simplified for filling logic used in nk5110_draw_rfrect
        if (corner == 1) { // Top Right
            if (y > 0) draw_fast_hLine(x0, y0 - y, x + 1, set); 
            if (x > 0 && x != y) draw_fast_hLine(x0, y0 - x, y + 1, set);
        } else if (corner == 2) { // Top Left
            if (y > 0) draw_fast_hLine(x0 - x, y0 - y, x + 1, set); 
            if (x > 0 && x != y) draw_fast_hLine(x0 - y, y0 - x, y + 1, set); 
        } else if (corner == 3) { // Bottom Left (0x4 equivalent for internal use)
            if (x > 0 && x != y) draw_fast_hLine(x0 - y, y0 + x, y + 1, set); 
            if (y > 0) draw_fast_hLine(x0 - x, y0 + y, x + 1, set); 
        } else if (corner == 4) { // Bottom Right (0x8 equivalent for internal use)
            if (x > 0 && x != y) draw_fast_hLine(x0, y0 + x, y + 1, set);
            if (y > 0) draw_fast_hLine(x0, y0 + y, x + 1, set); 
        }

        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
    }
}


// --- Drawing Implementations ---

void nk5110_draw_line(int x1, int y1, int x2, int y2, uint8_t set) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    for (;;) {
        nk5110_set_pixel(x1, y1, set);
        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y1 += sy;
        }
    }
}

void nk5110_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t set) {
    if (w < 1 || h < 1) return;
    uint8_t x2 = x + w - 1;
    uint8_t y2 = y + h - 1;

    nk5110_draw_line(x, y, x2, y, set);    // Top
    nk5110_draw_line(x2, y, x2, y2, set);  // Right
    nk5110_draw_line(x2, y2, x, y2, set);  // Bottom
    nk5110_draw_line(x, y2, x, y, set);    // Left
}

void nk5110_draw_frect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t set) {
    for (int16_t i = y; i < y + h; i++) {
        draw_fast_hLine(x, i, w, set);
    }
}

void nk5110_draw_rfrect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t set) {
    if (r > w/2 || r > h/2) r = (w < h ? w : h) / 2;

    // Fill the center rectangle (straight part)
    nk5110_draw_frect(x, y + r, w, h - 2 * r, set); 
    
    // Fill the top/bottom rectangular extensions 
    nk5110_draw_frect(x + r, y, w - 2 * r, r, set);   
    nk5110_draw_frect(x + r, y + h - r, w - 2 * r, r, set);  

    // Fill the four corners (uses corner flags 1-4 for simplified fill logic)
    fill_round_circle_quarter(x + w - r - 1, y + r, r, 1, set);       // Top Right
    fill_round_circle_quarter(x + r, y + r, r, 2, set);           // Top Left
    fill_round_circle_quarter(x + r, y + h - r - 1, r, 3, set);       // Bottom Left
    fill_round_circle_quarter(x + w - r - 1, y + h - r - 1, r, 4, set); // Bottom Right
}

void nk5110_draw_rrect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t set) {
    if (r > w/2 || r > h/2) r = (w < h ? w : h) / 2;

    // 1. Draw horizontal lines for the top and bottom straight sections
    draw_fast_hLine(x + r, y, w - 2 * r, set);             // Top straight
    draw_fast_hLine(x + r, y + h - 1, w - 2 * r, set);         // Bottom straight
    
    // 2. Draw vertical lines for the left and right straight sections 
    nk5110_draw_frect(x, y + r, 1, h - 2 * r, set);              // Left straight (vertical line, 1 pixel wide)
    nk5110_draw_frect(x + w - 1, y + r, 1, h - 2 * r, set);          // Right straight (vertical line, 1 pixel wide)
    
    // 3. Draw the four quarter-circles (uses bitmasks 1, 2, 4, 8)
    draw_round_circle_quarter(x + w - r - 1, y + r, r, 0x1, set);       // Top Right
    draw_round_circle_quarter(x + r, y + r, r, 0x2, set);           // Top Left
    draw_round_circle_quarter(x + r, y + h - r - 1, r, 0x4, set);       // Bottom Left
    draw_round_circle_quarter(x + w - r - 1, y + h - r - 1, r, 0x8, set); // Bottom Right
}

void nk5110_draw_char(uint8_t x, uint8_t y, char c, uint8_t set) {
    uint8_t index = c - FONT_START_CHAR;
    if (index >= FONT_TABLE_SIZE) index = 0; 

    const uint8_t *bitmap = font_table[index];

    for (int col = 0; col < FONT_CHAR_WIDTH; col++) {
        uint8_t column_data = bitmap[col];
        for (int row = 0; row < FONT_CHAR_HEIGHT; row++) {
            if (column_data & (1 << row)) {
                nk5110_set_pixel(x + col, y + row, set);
            } 
        }
    }
}

void nk5110_draw_str(uint8_t x, uint8_t y, const char *str, uint8_t set) {
    uint8_t cursor_x = x;
    for (int i = 0; str[i] != '\0'; i++) {
        if (cursor_x + FONT_CHAR_WIDTH >= LCD_WIDTH) break; 
        nk5110_draw_char(cursor_x, y, str[i], set);
        cursor_x += FONT_CHAR_WIDTH + 1; // 1 pixel space between characters
    }
}

void nk5110_draw_bitmap(uint8_t x_start, uint8_t y_start, uint8_t w, uint8_t h, const uint8_t *bitmap, uint8_t set) {
    const uint8_t h_banks = (w + 7) / 8; // Calculate bytes per row (e.g., 20 pixels wide = 3 bytes)
    uint16_t data_index = 0;

    for (uint8_t row = 0; row < h; row++) { 
        uint8_t global_y = y_start + row;
        if (global_y >= LCD_HEIGHT) {
             data_index += h_banks; 
             continue;
        }

        for (uint8_t h_bank = 0; h_bank < h_banks; h_bank++) { 
            
            // Check to avoid reading past the end of the bitmap array
            if (data_index >= (w * h_banks)) break; 
            
            uint8_t bank_data = bitmap[data_index++]; 
            uint8_t bank_start_x = x_start + (h_bank * 8);

            for (uint8_t bit_col = 0; bit_col < 8; bit_col++) {
                uint8_t global_x = bank_start_x + bit_col;
                
                if (global_x >= LCD_WIDTH || global_x >= x_start + w) continue;
                
                if (bank_data & (1 << bit_col)) {
                    nk5110_set_pixel(global_x, global_y, set);
                }
            }
        }
    }
}


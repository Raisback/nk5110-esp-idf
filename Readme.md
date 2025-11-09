## PCD8544 (Nokia 5110 LCD) Driver for ESP-IDF

This library provides a set of high-level functions for initializing the PCD8544 controller and efficiently drawing graphics and text to its 84x48 pixel monochromatic display using a fast internal frame buffer. All coordinates use the standard Cartesian system, with (0, 0) being the top-left corner

### Installation

This driver is designed to be used as a component within your ESP-IDF project's components directory.

__Step 1: Clone the Repository__

Clone this repository into the components folder of your main ESP-IDF project:

`cd YOUR_ESP_IDF_PROJECT/components`   
`git clone https://github.com/YOUR_USERNAME/pcd8544-driver.git nk5110_driver`

__Step 2: Configure the Project__

Ensure your project's main CMakeLists.txt or the component's CMakeLists.txt is configured to link against the necessary spi driver


### Core API Reference

__Initialization & Control__

  `nk5110_init()`                        Initializes the GPIO, SPI bus, performs hardware reset, and clears the display.  
  
  `void nk5110_update_display()`         Sends the entire 504-byte frame buffer contents to the physical LCD module     
  
  `nk5110_fill_buffer(uint8_t pattern)`  Fills the internal frame buffer. Use 0x00 to clear (all pixels OFF/White) or 0xFF to fill solid (all pixels ON/Black).  
  

  
  __Pixel and Primitive Drawing__

  ``nk5110_set_pixel(x, y, set)``  Sets or clears a single pixel in the frame buffer |  _x: 0 to 83, y: 0 to 47_


  ``nk5110_draw_line(x1, y1, x2, y2, set)``	 Draws a line segment between two specified points | _x1, y1, x2, y2 (integer coordinates).)_
  
  ``nk5110_draw_rect(x, y, w, h, set)``	Draws the outline of a rectangular box | _x, y (top-left), w (width), h (height)._ 
	
  ``nk5110_draw_frect(x, y, w, h, set)``	Draws a solid filled rectangular box |  _x, y (top-left), w (width), h (height)._ 

  ``nk5110_draw_rrect(x, y, w, h, r, set)``	Draws the outline of a rectangle with rounded corners | _r is the radius of the rounded corners._ 
	
  ``nk5110_draw_rfrect(x, y, w, h, r, set)``	Draws a solid filled rectangle with rounded corners | _r is the radius of the rounded corners._ 

__Text and Bitmap Drawing__

`nk5110_draw_char(x, y, c, set)`   Draws a single 5x8 pixel character. | _c: The ASCII character to draw (32 to 126 supported)._

`nk5110_draw_str(x, y, str, set)`	 Draws a null-terminated string using the built-in 5x8 font. | _str: Pointer to the string._

`nk5110_draw_bitmap(x, y, w, h, bitmap, set)`  Draws a monochromatic bitmap image defined by a raw byte array. | _bitmap: Pointer to the raw byte array (must be column-major)._

## Code Example

```
#include "nk5110.h"

// Example: A simple 8x8 bitmap (a diagonal line)
const uint8_t diag_8x8_bitmap[] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 // 8 bytes, 8x8 pixels
};

void draw_all_examples() {
    // 1. Initialize the display (Must be called once in app_main)
    // nk5110_init(); 
    
    // 2. Clear the screen (set buffer to all 0x00 / white background)
    nk5110_fill_buffer(0x00); 

    // Primitive Drawing Examples
    nk5110_draw_line(10, 5, 30, 20, 1);       // Draw a line 
    nk5110_draw_frect(55, 5, 8, 12, 1);      // Draw a filled rectangle 
    nk5110_draw_rrect(65, 5, 15, 10, 3, 1);   // Draw a rounded outline rectangle (r=3) 
    
    // Text and Bitmap Drawing Examples
    nk5110_draw_char(2, 25, 'E', 1);          // Draw the letter 'E' 
    nk5110_draw_str(10, 25, "HELLO WORLD", 1); // Draw a string 
    nk5110_draw_bitmap(40, 35, 8, 8, diag_8x8_bitmap, 1); // Draw the 8x8 bitmap

    // 3. Update the display to show all the drawn elements (MANDATORY)
    nk5110_update_display(); 
}
```

## License

This project is licensed under the [MIT License](LICENSE).

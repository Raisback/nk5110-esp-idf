## ESP-IDF PCD8544 LCD Driver

A dedicated and efficient ESP-IDF component for interfacing with the PCD8544 LCD controller, commonly found on the popular Nokia 5110 (84x48) display modules.

This driver is written in C and leverages the ESP-IDF's native SPI Master driver for high-speed, reliable communication, making it ideal for graphics-intensive applications on ESP32, ESP32-S2, ESP32-S3, and other ESP chips.

### Key Features

Native ESP-IDF Component: Easily integrates into any ESP-IDF project structure.

SPI Master Driver: Utilizes hardware SPI for maximum data throughput.

Full Frame Buffer: Implements a complete 504-byte in-memory frame buffer to prevent display flickering during complex draw operations. Updates are pushed only when explicitly commanded.

Graphics Primitives: Supports core functions like setting individual pixels, drawing lines, and rendering bitmaps.

Customizable GPIO: All control pins (DC, CE, RST) are configurable via the API

Text and Fonts: Includes support for drawing text using a built-in or custom font system.



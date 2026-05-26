// OLED display driver for SSD1306-based displays
// Using the U8G2 library for text and graphics rendering
#include "include/display.h"

void display_init(u8g2_t *u8g2, uint8_t address)
{
    u8g2_hal_init();
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        u8g2, // struct to initialize
        U8G2_R0, // no rotation, landscape by default
        u8g2_byte_hw_i2c, // function pointer for hardware i2c callback function
        u8g2_gpio_and_delay_cb // function pointer for gpio and delay callback function
    );


    u8g2_SetI2CAddress(u8g2, 0x3c);
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);
}

void display_draw_hello(u8g2_t *u8g2, uint8_t inc)
{
    u8g2_ClearBuffer(u8g2);
    u8g2_SetFont(u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(u8g2, 0, 15*inc, "Hello Pico!");
    u8g2_SendBuffer(u8g2);
}




// OLED display driver for SSD1306-based displays
// Using the U8G2 library for text and graphics rendering
#include <stdbool.h>
#include "include/display.h"

bool display_init(u8g2_t *u8g2, uint8_t *address)
{
    u8g2_hal_init();
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8g2_byte_hw_i2c, u8g2_gpio_and_delay_cb);

    uint8_t detected_address = u8g2_hal_detect_i2c_address();
    bool display_found = detected_address != 0;
    uint8_t use_address = display_found ? detected_address : 0x3C;

    u8g2_SetI2CAddress(u8g2, use_address);
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);

    if (address) {
        *address = use_address;
    }
    return display_found;
}

void display_draw_hello(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2);
    u8g2_SetFont(u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(u8g2, 0, 15, "Hello Pico!");
    u8g2_SendBuffer(u8g2);
}


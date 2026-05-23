#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "u8g2.h"
#include <string.h>


#define I2C_PORT i2c0
#define I2C_SDA 20
#define I2C_SCL 21
#define SSD1306_I2C_ADDR_3C 0x3C
#define SSD1306_I2C_ADDR_3D 0x3D

// Initialize the hardware I2C port used by the SSD1306.
void u8g2_hal_init(void) {
    i2c_init(I2C_PORT, 400 * 1000);  // 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

static bool u8g2_hal_probe_address(uint8_t address) {
    // Probe the bus to detect whether an SSD1306 is present at this address.
    const uint8_t probe_byte = 0x00;
    int ret = i2c_write_blocking(I2C_PORT, address, &probe_byte, 1, false);
    return ret >= 0;
}

uint8_t u8g2_hal_detect_i2c_address(void) {
    if (u8g2_hal_probe_address(SSD1306_I2C_ADDR_3D)) {
        return SSD1306_I2C_ADDR_3D;
    }
    if (u8g2_hal_probe_address(SSD1306_I2C_ADDR_3C)) {
        return SSD1306_I2C_ADDR_3C;
    }
    return 0; // not found
}

// This is called by u8g2 to send commands/data
static uint8_t u8g2_i2c_tx_buffer[128];
static size_t u8g2_i2c_tx_len = 0;

uint8_t u8g2_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_val, void *arg_ptr) {
    uint8_t i2c_address = u8x8_GetI2CAddress(u8x8);
    switch(msg) {
        case U8X8_MSG_BYTE_INIT:
            u8g2_hal_init();
            return 1;
        case U8X8_MSG_BYTE_START_TRANSFER:
            u8g2_i2c_tx_len = 0;
            return 1;
        case U8X8_MSG_BYTE_END_TRANSFER:
            if (u8g2_i2c_tx_len > 0) {
                int ret = i2c_write_blocking(I2C_PORT, i2c_address, u8g2_i2c_tx_buffer, u8g2_i2c_tx_len, false);
                u8g2_i2c_tx_len = 0;
                return ret >= 0;
            }
            return 1;
        case U8X8_MSG_BYTE_SEND:
            if (arg_val > 0) {
                if (u8g2_i2c_tx_len + arg_val > sizeof(u8g2_i2c_tx_buffer)) {
                    return 0;
                }
                memcpy(&u8g2_i2c_tx_buffer[u8g2_i2c_tx_len], arg_ptr, arg_val);
                u8g2_i2c_tx_len += arg_val;
            }
            return 1;
        case U8X8_MSG_BYTE_SET_DC:
            return 1;
    }
    return 1;
}

// Delays and GPIO callback
uint8_t u8g2_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_val, void *arg_ptr) {
    (void)u8x8;
    (void)arg_ptr;
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            //u8g2_hal_init();
            break;
        case U8X8_MSG_DELAY_NANO:
            if (arg_val >= 1000) {
                sleep_us((arg_val + 999) / 1000);
            }
            break;
        case U8X8_MSG_DELAY_10MICRO:
            sleep_us(arg_val * 10);
            break;
        case U8X8_MSG_DELAY_100NANO:
            if (arg_val >= 100) {
                sleep_us((arg_val + 99) / 100);
            }
            break;
        case U8X8_MSG_DELAY_MILLI:
            sleep_ms(arg_val);
            break;
    }
    return 1;
}
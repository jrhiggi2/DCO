#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "u8g2.h"

void u8g2_hal_init(void);
uint8_t u8g2_hal_detect_i2c_address(void);
uint8_t u8g2_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_val, void *arg_ptr);
uint8_t u8g2_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_val, void *arg_ptr);

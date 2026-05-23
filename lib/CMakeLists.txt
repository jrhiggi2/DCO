# lib/CMakeLists.txt
add_library(u8g2 STATIC
    u8g2/csrc/u8g2_d_setup.c
    u8g2/csrc/u8g2_d_memory.c
    u8g2/csrc/u8g2_fonts.c
    u8g2/csrc/u8g2_font.c
    u8g2/csrc/u8g2_intersection.c
    u8g2/csrc/u8g2_setup.c
    u8g2/csrc/u8g2_buffer.c
    u8g2/csrc/u8g2_ll_hvline.c
    u8g2/csrc/u8g2_hvline.c
    u8g2/csrc/u8x8_byte.c
    u8g2/csrc/u8x8_cad.c
    u8g2/csrc/u8x8_display.c
    u8g2/csrc/u8x8_setup.c
    u8g2/csrc/u8x8_message.c
    u8g2/csrc/u8x8_gpio.c
    u8g2/csrc/u8x8_8x8.c
    u8g2/csrc/u8x8_string.c
    u8g2/csrc/u8x8_u8toa.c
    u8g2/csrc/u8x8_u16toa.c
    u8g2/csrc/u8x8_d_ssd1306_128x64_noname.c
    u8g2_hal.c
)

target_include_directories(u8g2 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/u8g2/csrc
)

target_link_libraries(u8g2 PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_gpio
)

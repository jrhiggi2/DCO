# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(DCO1 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(DCO1 
        src/main.c
        src/adc.c
        src/dma.c
        src/gpio.c
        src/pio.c
        src/pwm.c
        src/uart.c
        src/timer.c
        src/adsr.c
        src/display.c
        src/gfx.c
        src/u8g2_hal.c
)

pico_set_program_name(DCO1 "DCO1")
pico_set_program_version(DCO1 "0.1")

# Generate PIO header
pico_generate_pio_header(DCO1 ${CMAKE_CURRENT_LIST_DIR}/osc_reset.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(DCO1 0)
pico_enable_stdio_usb(DCO1 1)

# Add u8g2 library from the lib folder
#add_subdirectory(lib)

# add condensed u8g2 library
add_library(u8g2 STATIC
    lib/u8g2/csrc/u8g2_d_setup.c
    lib/u8g2/csrc/u8g2_d_memory.c
    lib/u8g2/csrc/u8g2_fonts.c
    lib/u8g2/csrc/u8g2_font.c
    lib/u8g2/csrc/u8g2_intersection.c
    lib/u8g2/csrc/u8g2_setup.c
    lib/u8g2/csrc/u8g2_buffer.c
    lib/u8g2/csrc/u8g2_ll_hvline.c
    lib/u8g2/csrc/u8g2_hvline.c
    lib/u8g2/csrc/u8x8_byte.c
    lib/u8g2/csrc/u8x8_cad.c
    lib/u8g2/csrc/u8x8_display.c
    lib/u8g2/csrc/u8x8_setup.c
    lib/u8g2/csrc/u8x8_message.c
    lib/u8g2/csrc/u8x8_gpio.c
    lib/u8g2/csrc/u8x8_8x8.c
    lib/u8g2/csrc/u8x8_string.c
    lib/u8g2/csrc/u8x8_u8toa.c
    lib/u8g2/csrc/u8x8_u16toa.c
    lib/u8g2/csrc/u8x8_d_ssd1306_128x64_noname.c
    lib/u8g2/csrc/u8g2_line.c
    lib/u8g2/csrc/u8g2_box.c
    lib/u8g2/csrc/u8g2_circle.c
    lib/u8g2/csrc/u8g2_bitmap.c
)

# find .h files associated with u8g2 library .c files
target_include_directories(u8g2 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/u8g2/csrc
)

# u8g2 library is dependent on pico_stdlib so it must be linked
target_link_libraries(u8g2 PUBLIC pico_stdlib)

# Target libraries
target_link_libraries(DCO1
        pico_stdlib
        hardware_dma
        hardware_pio
        hardware_adc
        hardware_pwm
        hardware_uart
        hardware_timer
        hardware_i2c
        hardware_gpio
        u8g2
        )

# Add the include files to the build
target_include_directories(DCO1 PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(DCO1)


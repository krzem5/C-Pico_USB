cmake_minimum_required(VERSION 3.12)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)
include(${PICO_SDK_INIT_CMAKE_FILE})
project(pico_usb LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_compile_options(-Wall -Wno-format -Werror -O3 -g0 -fdata-sections -ffunction-sections -Wl,--gc-sections)
include_directories(src/include)
add_executable(pico_usb
	src/main.c
	src/usb/fat16.c
	src/usb/hid_keyboard.c
	src/usb/msc.c
	src/usb/usb.c
)
pico_enable_stdio_usb(pico_usb 0)
pico_enable_stdio_uart(pico_usb 0)
target_link_libraries(pico_usb pico_stdlib hardware_resets hardware_irq hardware_gpio)
pico_add_extra_outputs(pico_usb)

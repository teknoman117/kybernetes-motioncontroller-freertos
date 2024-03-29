cmake_minimum_required(VERSION 3.20)

include(cmake/avr.cmake)

project(freertos-test VERSION 0.1)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

set(F_CPU 8000000UL)
set(F_I2C 333333UL)
set(MCU atmega644)
set(BAUD 57600)
set(RXBUFFER_SIZE 128)
set(TXBUFFER_SIZE 128)

add_definitions(-DF_CPU=${F_CPU} -DBAUD=${BAUD} -DF_I2C=${F_I2C} -DRXBUFFER_SIZE=${RXBUFFER_SIZE} -DTXBUFFER_SIZE=${TXBUFFER_SIZE})
add_compile_options(-mmcu=${MCU})
add_link_options(-mmcu=${MCU})

add_subdirectory(dependencies)
add_subdirectory(src)
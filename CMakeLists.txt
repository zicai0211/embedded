cmake_minimum_required(VERSION 3.16)
project(stm32_demo C ASM)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX arm-none-eabi-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)

set(MCU_FLAGS
        -mcpu=cortex-m4
        -mthumb
)

include_directories(
        ${CMAKE_SOURCE_DIR}/app/inc
        ${CMAKE_SOURCE_DIR}/uos/inc
        ${CMAKE_SOURCE_DIR}/driver/inc
)

file(GLOB_RECURSE SOURCES
        ${CMAKE_SOURCE_DIR}/app/*.c
        ${CMAKE_SOURCE_DIR}/uos/src/*.c
        ${CMAKE_SOURCE_DIR}/driver/src/*.c
)

set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/src/startup_stm32f40xx.s)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F407VGTx_FLASH.ld)

add_executable(${PROJECT_NAME}.elf
        ${SOURCES}
        ${STARTUP_FILE}
)

target_compile_options(${PROJECT_NAME}.elf PRIVATE
        ${MCU_FLAGS}
        -O0
        -g
        -ffunction-sections
        -fdata-sections
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
        ${MCU_FLAGS}
        -T${LINKER_SCRIPT}
        -Wl,--gc-sections
        -Wl,-Map=${PROJECT_NAME}.map
)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex   ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
        COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
)
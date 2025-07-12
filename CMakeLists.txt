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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(FOCProject C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(FOCProject 
                FOCProject.cpp 
                MT6701/MT6701.cpp
                )

pico_set_program_name(FOCProject "FOCProject")
pico_set_program_version(FOCProject "0.1")

# 生成 PIO 头文件
# 更改 pico_generate_pio_header 的第二个参数，指定生成的头文件路径。
# 这里将输出路径设置为项目根目录下的 MT6701/ 目录，文件名仍为 MT6701.pio.h
pico_generate_pio_header(FOCProject ${CMAKE_SOURCE_DIR}/MT6701/MT6701.pio)



# Modify the below lines to enable/disable output over UART/USB

pico_enable_stdio_usb(FOCProject 1)
pico_enable_stdio_uart(FOCProject 0)

# Add the standard library to the build
target_link_libraries(FOCProject
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(FOCProject PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        
)

# Add any user requested libraries
target_link_libraries(FOCProject 
        hardware_spi
        hardware_pio
        hardware_dma
        pico_stdlib
        )

pico_add_extra_outputs(FOCProject)


cmake_minimum_required(VERSION 3.20)

include("${CMAKE_CURRENT_LIST_DIR}/../../common/cmake/gcc-milandr.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/usb_examples_common.cmake")
set(CMAKE_PROJECT_NAME synt)
project(${CMAKE_PROJECT_NAME} ASM C CXX)

usb_examples_add_milandr_sdk()

add_executable(${CMAKE_PROJECT_NAME})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "app/inc"
)

usb_examples_apply_common_with_gpio(${CMAKE_PROJECT_NAME})
usb_examples_apply_freertos(${CMAKE_PROJECT_NAME})
usb_examples_apply_milandr_sdk(${CMAKE_PROJECT_NAME})

get_filename_component(USB_EXAMPLES_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    "app/src/app.c"
    "app/src/app_usb_midi.c"
    "app/src/app_dac_synth.c"
    "app/src/dma_control_table.c"
    "${USB_EXAMPLES_ROOT}/common/Drivers/SPL/src/MDR32FxQI_dac.c"
    "${USB_EXAMPLES_ROOT}/common/Drivers/SPL/src/MDR32FxQI_dma.c"
    "${USB_EXAMPLES_ROOT}/common/Drivers/SPL/src/MDR32FxQI_timer.c"
)

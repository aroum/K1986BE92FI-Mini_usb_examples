cmake_minimum_required(VERSION 3.20)

include("${CMAKE_CURRENT_LIST_DIR}/../../common/cmake/gcc-milandr.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/usb_examples_common.cmake")
set(CMAKE_PROJECT_NAME combo_usb)
project(${CMAKE_PROJECT_NAME} ASM C CXX)

set(MILANDR_SDK_USB_CDC ON)
usb_examples_add_milandr_sdk()

add_executable(${CMAKE_PROJECT_NAME})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "app/inc"
)

usb_examples_apply_common_with_gpio(${CMAKE_PROJECT_NAME})
usb_examples_apply_freertos(${CMAKE_PROJECT_NAME})
usb_examples_apply_milandr_sdk(${CMAKE_PROJECT_NAME})

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    "app/src/app.c"
    "app/src/app_usb_hid.c"
    "app/src/app_combo_dispatch.c"
)

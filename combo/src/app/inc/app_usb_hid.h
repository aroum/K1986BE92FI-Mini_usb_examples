/**
 * @file    app_usb_hid.h
 * @brief   USB HID Keyboard driver definitions and API.
 */

#ifndef __APP_USB_HID_H
#define __APP_USB_HID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MDR32FxQI_config.h"
#include "MDR32FxQI_usb_device.h"

/* HID Descriptor Types */
#define USB_HID_DESCRIPTOR_HID           0x21
#define USB_HID_DESCRIPTOR_REPORT        0x22

/* HID Class Requests */
#define USB_HID_GET_REPORT               0x01
#define USB_HID_GET_IDLE                 0x02
#define USB_HID_GET_PROTOCOL             0x03
#define USB_HID_SET_REPORT               0x09
#define USB_HID_SET_IDLE                 0x0A
#define USB_HID_SET_PROTOCOL             0x0B

/* Keyboard Modifier Bitmasks */
#define KEYBOARD_MODIFIER_LEFTCTRL       (1 << 0)
#define KEYBOARD_MODIFIER_LEFTSHIFT      (1 << 1)
#define KEYBOARD_MODIFIER_LEFTALT        (1 << 2)
#define KEYBOARD_MODIFIER_LEFTGUI        (1 << 3)
#define KEYBOARD_MODIFIER_RIGHTCTRL      (1 << 4)
#define KEYBOARD_MODIFIER_RIGHTSHIFT     (1 << 5)
#define KEYBOARD_MODIFIER_RIGHTALT       (1 << 6)
#define KEYBOARD_MODIFIER_RIGHTGUI       (1 << 7)

/* Common Keyboard Usages */
#define KEY_F                            0x09
#define KEY_A                            0x04
#define KEY_B                            0x05
#define KEY_C                            0x06
#define KEY_D                            0x07
#define KEY_E                            0x08
#define KEY_G                            0x0A
#define KEY_H                            0x0B
#define KEY_I                            0x0C
#define KEY_J                            0x0D
#define KEY_K                            0x0E
#define KEY_L                            0x0F
#define KEY_M                            0x10
#define KEY_N                            0x11
#define KEY_O                            0x12
#define KEY_P                            0x13
#define KEY_Q                            0x14
#define KEY_R                            0x15
#define KEY_S                            0x16
#define KEY_T                            0x17
#define KEY_U                            0x18
#define KEY_V                            0x19
#define KEY_W                            0x1A
#define KEY_X                            0x1B
#define KEY_Y                            0x1C
#define KEY_Z                            0x1D

/* USB Endpoint Assignments for HID */
#define USB_HID_EP_SEND                  USB_EP1

#pragma pack(push, 1)
/**
 * @brief Standard 8-byte HID keyboard input report.
 */
typedef struct
{
    uint8_t Modifier;   /* Modifier keys bitmask */
    uint8_t Reserved;   /* Reserved byte, must be 0 */
    uint8_t Keycodes[6];/* Up to 6 concurrent keycodes */
} USB_HID_KeyboardReport_TypeDef;
#pragma pack(pop)

/**
 * @brief  Initializes the USB HID Keyboard.
 * @retval USB_Result.
 */
USB_Result USB_HID_Init(void);

/**
 * @brief  Sends a keyboard input report to the host via Interrupt EP1.
 * @param  report: Pointer to the populated report structure.
 * @retval USB_Result.
 */
USB_Result USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef* report);

/**
 * @brief  USB device reset handler to reconfigure HID endpoints.
 * @retval USB_Result.
 */
USB_Result USB_HID_Reset(void);

/**
 * @brief  Standard request GET_DESCRIPTOR handler for HID specific descriptors.
 * @param  wVALUE: Descriptor type (high byte) and index (low byte).
 * @param  wINDEX: Language ID or Interface number.
 * @param  wLENGTH: Maximum number of bytes requested by host.
 * @retval USB_Result.
 */
USB_Result USB_HID_GetDescriptor(uint16_t wVALUE, uint16_t wINDEX, uint16_t wLENGTH);

/**
 * @brief  Class request handler to process HID requests (GET/SET REPORT/IDLE/PROTOCOL).
 * @retval USB_Result.
 */
USB_Result USB_HID_ClassRequest(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_USB_HID_H */

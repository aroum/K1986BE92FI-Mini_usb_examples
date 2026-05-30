/**
  ******************************************************************************
  * @file    MDR32FxQI_usb_handlers.h
  * @brief   Combo example: CDC or HID handlers selected at boot.
  ******************************************************************************
  */

#ifndef __MDR32FxQI_USB_HANDLERS_H
#define __MDR32FxQI_USB_HANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MDR32FxQI_usb_default_handlers.h"

#ifndef __MDR32FxQI_CONFIG_H
    #error "Configuration file MDR32FxQI_config.h should be included before"
#endif

#include "app_combo.h"

#undef USB_CDC_HANDLE_DATA_RECEIVE
#define USB_CDC_HANDLE_DATA_RECEIVE(BUFFER, LENGTH) \
    (App_ComboCdcMode() ? USB_CDC_RecieveData(BUFFER, LENGTH) : USB_ERROR)

#ifdef USB_CDC_LINE_CODING_SUPPORTED
    #undef USB_CDC_HANDLE_GET_LINE_CODING
    #define USB_CDC_HANDLE_GET_LINE_CODING(wINDEX, DATA) \
        (App_ComboCdcMode() ? USB_CDC_GetLineCoding(wINDEX, DATA) : USB_ERROR)
    #undef USB_CDC_HANDLE_SET_LINE_CODING
    #define USB_CDC_HANDLE_SET_LINE_CODING(wINDEX, DATA) \
        (App_ComboCdcMode() ? USB_CDC_SetLineCoding(wINDEX, DATA) : USB_ERROR)
#endif

#undef USB_DEVICE_HANDLE_RESET
#define USB_DEVICE_HANDLE_RESET App_ComboUsbReset()

#undef USB_DEVICE_HANDLE_GET_DESCRIPTOR
#define USB_DEVICE_HANDLE_GET_DESCRIPTOR(wVALUE, wINDEX, wLENGTH) \
    App_ComboUsbGetDescriptor(wVALUE, wINDEX, wLENGTH)

#undef USB_DEVICE_HANDLE_CLASS_REQUEST
#define USB_DEVICE_HANDLE_CLASS_REQUEST App_ComboUsbClassRequest()

USB_Result USB_CDC_RecieveData(uint8_t* Buffer, uint32_t Length);

#ifdef USB_CDC_LINE_CODING_SUPPORTED
    USB_Result USB_CDC_GetLineCoding(uint16_t wINDEX, USB_CDC_LineCoding_TypeDef* DATA);
    USB_Result USB_CDC_SetLineCoding(uint16_t wINDEX, const USB_CDC_LineCoding_TypeDef* DATA);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MDR32FxQI_USB_HANDLERS_H */

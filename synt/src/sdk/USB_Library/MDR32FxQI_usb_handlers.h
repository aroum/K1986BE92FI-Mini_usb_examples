/**
  ******************************************************************************
  * @file    MDR32FxQI_usb_handlers.h
  * @brief   USB handlers for synt (USB MIDI receive).
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

#include "app_usb_midi.h"

#undef USB_DEVICE_HANDLE_RESET
#define USB_DEVICE_HANDLE_RESET  USB_MIDI_Reset()

#undef USB_DEVICE_HANDLE_SET_CONFIGURATION
#define USB_DEVICE_HANDLE_SET_CONFIGURATION(wVALUE) \
    ((wVALUE) == 1 ? USB_MIDI_ConfigureEndpoints() : USB_ERROR)

#undef USB_DEVICE_HANDLE_GET_DESCRIPTOR
#define USB_DEVICE_HANDLE_GET_DESCRIPTOR(wVALUE, wINDEX, wLENGTH) \
    USB_MIDI_GetDescriptor(wVALUE, wINDEX, wLENGTH)

#undef USB_DEVICE_HANDLE_CLASS_REQUEST
#define USB_DEVICE_HANDLE_CLASS_REQUEST  USB_MIDI_ClassRequest()

#ifdef __cplusplus
}
#endif

#endif /* __MDR32FxQI_USB_HANDLERS_H */

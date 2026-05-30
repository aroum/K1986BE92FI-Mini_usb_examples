/**
 * @file    app_combo.h
 * @brief   USB mode selection for combo example (CDC vs HID keyboard).
 */

#ifndef APP_COMBO_H
#define APP_COMBO_H

#include <stdbool.h>
#include "MDR32FxQI_usb_device.h"

void App_ComboSetCdcMode(bool cdc_mode);
bool App_ComboCdcMode(void);

USB_Result App_ComboUsbReset(void);
USB_Result App_ComboUsbGetDescriptor(uint16_t wVALUE, uint16_t wINDEX, uint16_t wLENGTH);
USB_Result App_ComboUsbClassRequest(void);

#endif /* APP_COMBO_H */

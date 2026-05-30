/**
 * @file    app_combo_dispatch.c
 * @brief   Runtime USB handler dispatch (CDC or HID).
 */

#include "app_combo.h"
#include "app_usb_hid.h"
#include "MDR32FxQI_usb_CDC.h"

static bool s_cdc_mode;

void App_ComboSetCdcMode(bool cdc_mode)
{
  s_cdc_mode = cdc_mode;
}

bool App_ComboCdcMode(void)
{
  return s_cdc_mode;
}

USB_Result App_ComboUsbReset(void)
{
  return s_cdc_mode ? USB_CDC_Reset() : USB_HID_Reset();
}

USB_Result App_ComboUsbGetDescriptor(uint16_t wVALUE, uint16_t wINDEX, uint16_t wLENGTH)
{
  return s_cdc_mode ? USB_CDC_GetDescriptor(wVALUE, wINDEX, wLENGTH)
                    : USB_HID_GetDescriptor(wVALUE, wINDEX, wLENGTH);
}

USB_Result App_ComboUsbClassRequest(void)
{
  return s_cdc_mode ? USB_CDC_ClassRequest() : USB_HID_ClassRequest();
}

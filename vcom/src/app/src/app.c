/**
 * @file    app.c
 * @brief   USB VCOM echo example (FreeRTOS).
 */

#include "app.h"
#include "MDR32FxQI_config.h"
#include "MDR32FxQI_usb_handlers.h"
#include "board_usb.h"

#define BUFFER_LENGTH 100

static uint8_t Buffer[BUFFER_LENGTH];

#ifdef USB_CDC_LINE_CODING_SUPPORTED
static USB_CDC_LineCoding_TypeDef LineCoding;
#endif

#ifdef USB_VCOM_SYNC
volatile uint32_t PendingDataLength = 0;
#endif

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
  (void)pxTask;
  (void)pcTaskName;
  while (1) {
    __NOP();
  }
}

void vApplicationIdleHook(void)
{
}

int main(void)
{
  CLK_Init_80_mhz();

#ifdef USB_CDC_LINE_CODING_SUPPORTED
  LineCoding.dwDTERate = 115200;
  LineCoding.bCharFormat = 0;
  LineCoding.bParityType = 0;
  LineCoding.bDataBits = 8;
#endif

  USB_CDC_Init(Buffer, 1, SET);
  Board_USB_Init(true);

  vTaskStartScheduler();

  while (1) {
    __NOP();
  }
  return 0;
}

USB_Result USB_CDC_RecieveData(uint8_t *Buffer, uint32_t Length)
{
  USB_Result result = USB_CDC_SendData(Buffer, Length);

#ifdef USB_VCOM_SYNC
  if (result != USB_SUCCESS) {
    PendingDataLength = Length;
  }
  return result;
#else
  (void)result;
  return USB_SUCCESS;
#endif
}

#ifdef USB_VCOM_SYNC
USB_Result USB_CDC_DataSent(void)
{
  if (PendingDataLength) {
    USB_CDC_SendData(Buffer, PendingDataLength);
    PendingDataLength = 0;
    USB_CDC_ReceiveStart();
  }
  return USB_SUCCESS;
}
#endif

#ifdef USB_CDC_LINE_CODING_SUPPORTED
USB_Result USB_CDC_GetLineCoding(uint16_t wINDEX, USB_CDC_LineCoding_TypeDef *DATA)
{
  if (wINDEX != 0) {
    return USB_ERR_INV_REQ;
  }
  *DATA = LineCoding;
  return USB_SUCCESS;
}

USB_Result USB_CDC_SetLineCoding(uint16_t wINDEX, const USB_CDC_LineCoding_TypeDef *DATA)
{
  if (wINDEX != 0) {
    return USB_ERR_INV_REQ;
  }
  LineCoding = *DATA;
  return USB_SUCCESS;
}
#endif

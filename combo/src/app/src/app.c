/**
 * @file    app.c
 * @brief   Combo USB example: hold USR button at boot for VCOM, else HID keyboard.
 */

#include "app.h"
#include "app_combo.h"
#include "app_usb_hid.h"
#include "board_gpio.h"
#include "board_usb.h"
#include "MDR32FxQI_config.h"
#include "MDR32FxQI_usb_handlers.h"

#define VCOM_BUFFER_LENGTH 100

static uint8_t VcomBuffer[VCOM_BUFFER_LENGTH];

#ifdef USB_CDC_LINE_CODING_SUPPORTED
static USB_CDC_LineCoding_TypeDef LineCoding;
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
  while (1) {
    __NOP();
  }
}

static void keyboardTask(void *pvParameters)
{
  (void)pvParameters;
  USB_HID_KeyboardReport_TypeDef report;
  uint8_t isKeyPressed = 0;

  report.Modifier = 0;
  report.Reserved = 0;
  for (int i = 0; i < 6; i++) {
    report.Keycodes[i] = 0;
  }

  vTaskDelay(pdMS_TO_TICKS(5000));

  for (;;) {
    if (Board_ButtonPressed()) {
      Board_LED_On();
      if (!isKeyPressed) {
        report.Keycodes[0] = KEY_F;
        USB_HID_SendReport(&report);
        isKeyPressed = 1;
      }
    } else {
      Board_LED_Off();
      if (isKeyPressed) {
        report.Keycodes[0] = 0;
        USB_HID_SendReport(&report);
        isKeyPressed = 0;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

static void initVcomMode(void)
{
#ifdef USB_CDC_LINE_CODING_SUPPORTED
  LineCoding.dwDTERate = 115200;
  LineCoding.bCharFormat = 0;
  LineCoding.bParityType = 0;
  LineCoding.bDataBits = 8;
#endif

  USB_CDC_Init(VcomBuffer, 1, SET);
  Board_USB_Init(true);
}

static void initKeyboardMode(void)
{
  USB_HID_Init();
  Board_USB_Init(false);
  xTaskCreate(keyboardTask, "kbdTask", configMINIMAL_STACK_SIZE * 2, NULL,
              tskIDLE_PRIORITY + 2, NULL);
}

int main(void)
{
  CLK_Init_80_mhz();
  Board_GPIO_Init();

  /* Button held at power-on → VCOM; released → HID keyboard */
  App_ComboSetCdcMode(Board_ButtonPressed());

  if (App_ComboCdcMode()) {
    initVcomMode();
  } else {
    initKeyboardMode();
  }

  vTaskStartScheduler();

  while (1) {
    __NOP();
  }
  return 0;
}

USB_Result USB_CDC_RecieveData(uint8_t *Buffer, uint32_t Length)
{
  if (!App_ComboCdcMode()) {
    return USB_ERROR;
  }
  (void)USB_CDC_SendData(Buffer, Length);
  return USB_SUCCESS;
}

#ifdef USB_CDC_LINE_CODING_SUPPORTED
USB_Result USB_CDC_GetLineCoding(uint16_t wINDEX, USB_CDC_LineCoding_TypeDef *DATA)
{
  if (!App_ComboCdcMode() || wINDEX != 0) {
    return USB_ERR_INV_REQ;
  }
  *DATA = LineCoding;
  return USB_SUCCESS;
}

USB_Result USB_CDC_SetLineCoding(uint16_t wINDEX, const USB_CDC_LineCoding_TypeDef *DATA)
{
  if (!App_ComboCdcMode() || wINDEX != 0) {
    return USB_ERR_INV_REQ;
  }
  LineCoding = *DATA;
  return USB_SUCCESS;
}
#endif

/**
 * @file    app.c
 * @brief   USB HID keyboard example.
 */

#include "app.h"
#include "board_gpio.h"
#include "board_usb.h"
#include "app_usb_hid.h"

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

int main(void)
{
  CLK_Init_80_mhz();
  Board_GPIO_Init();
  USB_HID_Init();
  Board_USB_Init(false);
  xTaskCreate(keyboardTask, "kbdTask", configMINIMAL_STACK_SIZE * 2, NULL,
              tskIDLE_PRIORITY + 2, NULL);
  vTaskStartScheduler();
  while (1) {
    __NOP();
  }
  return 0;
}

/**
 * @file    app.c
 * @brief   USB MIDI example.
 */

#include "app.h"
#include "board_gpio.h"
#include "board_usb.h"
#include "app_usb_midi.h"
#include "MDR32FxQI_usb_handlers.h"

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

static void midiTask(void *pvParameters)
{
  (void)pvParameters;
  uint8_t isKeyPressed = 0;
  const uint8_t midiChannel = 0;
  const uint8_t noteA4 = 69;
  const uint8_t maxVelocity = 127;

  vTaskDelay(pdMS_TO_TICKS(5000));

  for (;;) {
    if (Board_ButtonPressed()) {
      Board_LED_On();
      if (!isKeyPressed &&
          USB_DeviceContext.USB_DeviceState == USB_DEV_STATE_CONFIGURED) {
        if (USB_MIDI_SendNoteOn(midiChannel, noteA4, maxVelocity) == USB_SUCCESS) {
          isKeyPressed = 1;
        }
      }
    } else {
      Board_LED_Off();
      if (isKeyPressed &&
          USB_DeviceContext.USB_DeviceState == USB_DEV_STATE_CONFIGURED) {
        if (USB_MIDI_SendNoteOff(midiChannel, noteA4, 0) == USB_SUCCESS) {
          isKeyPressed = 0;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

int main(void)
{
  CLK_Init_80_mhz();
  Board_GPIO_Init();
  USB_MIDI_Init();
  Board_USB_Init(true);
  xTaskCreate(midiTask, "midiTask", configMINIMAL_STACK_SIZE * 4, NULL,
              tskIDLE_PRIORITY + 2, NULL);
  vTaskStartScheduler();
  while (1) {
    __NOP();
  }
  return 0;
}

/**
 * @file    app.c
 * @brief   USB VCOM echo + EEPROM example (FreeRTOS).
 */


#include "app.h"
#include "MDR32FxQI_config.h"
#include "MDR32FxQI_usb_handlers.h"
#include "board_usb.h"
#include "queue.h"
#include "MDR32FxQI_eeprom.h" 

#define BUFFER_LENGTH 100

#define EEPROM_TEST_ADDR 0x0801F000

static uint8_t Buffer[BUFFER_LENGTH];

QueueHandle_t xCharQueue;

#ifdef USB_CDC_LINE_CODING_SUPPORTED
static USB_CDC_LineCoding_TypeDef LineCoding;
#endif

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
  (void)pxTask;
  (void)pcTaskName;
  while (1)
  {
    __NOP();
  }
}

void vApplicationIdleHook(void)
{
}

void vEepromTask(void *pvParameters)
{
  uint8_t rxChar;
  uint8_t txBuf[6];
  uint32_t readData;

  RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);

  while (1)
  {
    if (xQueueReceive(xCharQueue, &rxChar, portMAX_DELAY) == pdTRUE)
    {
      /* Входим в критическую секцию: отключаем все прерывания */
      taskENTER_CRITICAL();

      /* 1. Согласно пункту 10.2.3 спецификации ТСКЯ.431296.037СП, Перед программированием ячейки необходимо выполнить ее стирание*/
      EEPROM_ErasePage(EEPROM_TEST_ADDR, EEPROM_Main_Bank_Select);

      /* 2. Записываем значение */
      EEPROM_ProgramWord(EEPROM_TEST_ADDR, EEPROM_Main_Bank_Select, (uint32_t) rxChar);

      // /* 3. Читаем значение */
      readData = EEPROM_ReadWord(EEPROM_TEST_ADDR, EEPROM_Main_Bank_Select);

      /* Выходим из критической секции */
      taskEXIT_CRITICAL();

      /* 4. Вывод результата */
      txBuf[0] = rxChar;
      txBuf[1] = '\r';
      txBuf[2] = '\n';
      txBuf[3] = (uint8_t)readData;
      txBuf[4] = '\r';
      txBuf[5] = '\n';

      while (USB_CDC_SendData(txBuf, 6) != USB_SUCCESS)
      {
        vTaskDelay(1);
      }
    }
  }
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

  /* Создаем очередь на 100 символов */
  xCharQueue = xQueueCreate(BUFFER_LENGTH, sizeof(uint8_t));

  /* Создаем задачу обработки ПЗУ */
  xTaskCreate(vEepromTask, "EEPROM_Task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);

  USB_CDC_Init(Buffer, 1, SET);
  Board_USB_Init(true);

  vTaskStartScheduler();

  while (1)
  {
    __NOP();
  }
  return 0;
}

USB_Result USB_CDC_RecieveData(uint8_t *Buffer, uint32_t Length)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  for (uint32_t i = 0; i < Length; i++)
  {
    xQueueSendFromISR(xCharQueue, &Buffer[i], &xHigherPriorityTaskWoken);
  }

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

  USB_CDC_ReceiveStart();

  return USB_SUCCESS;
}

#ifdef USB_VCOM_SYNC
USB_Result USB_CDC_DataSent(void)
{
  /* Перезапускаем прием после успешной отправки пакета */
  USB_CDC_ReceiveStart();
  return USB_SUCCESS;
}
#endif

#ifdef USB_CDC_LINE_CODING_SUPPORTED
USB_Result USB_CDC_GetLineCoding(uint16_t wINDEX, USB_CDC_LineCoding_TypeDef *DATA)
{
  if (wINDEX != 0)
  {
    return USB_ERR_INV_REQ;
  }
  *DATA = LineCoding;
  return USB_SUCCESS;
}

USB_Result USB_CDC_SetLineCoding(uint16_t wINDEX, const USB_CDC_LineCoding_TypeDef *DATA)
{
  if (wINDEX != 0)
  {
    return USB_ERR_INV_REQ;
  }
  LineCoding = *DATA;
  return USB_SUCCESS;
}
#endif
/**
 * @file    app.c
 * @brief   Combo USB example: hold USR button at boot for VCOM, else HID
 * keyboard.
 */

#include "app.h"
#include "MDR32FxQI_config.h"
#include "MDR32FxQI_eeprom.h"
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_usb_handlers.h"
#include "app_combo.h"
#include "app_usb_hid.h"
#include "board_gpio.h"
#include "board_usb.h"
#include "queue.h"

QueueHandle_t xCharQueue;

#define VCOM_BUFFER_LENGTH 100
#define EEPROM_STORAGE_ADDR 0x0801F000
#define DEFAULT_CHAR 'f'

static uint8_t VcomBuffer[VCOM_BUFFER_LENGTH];
static uint8_t storedChar = DEFAULT_CHAR;
static uint8_t receivedFlag = 0;

#ifdef USB_CDC_LINE_CODING_SUPPORTED
static USB_CDC_LineCoding_TypeDef LineCoding;
#endif

static uint8_t charToHIDCode(char c) {
  if (c >= 'a' && c <= 'z') {
    return KEY_A + (c - 'a');
  } else if (c >= 'A' && c <= 'Z') {
    return KEY_A + (c - 'A');
  }
  return KEY_F;
}

static char loadCharFromEEPROM(void) {
  RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
  EEPROM_SetLatency(EEPROM_Latency_3);
  uint32_t data = EEPROM_ReadWord(EEPROM_STORAGE_ADDR, EEPROM_Main_Bank_Select);
  RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, DISABLE);
  if (data == 0xFFFFFFFF) {
    return DEFAULT_CHAR;
  }
  if ((data & 0xFF) >= 'a' && (data & 0xFF) <= 'z') {
    return (char)(data & 0xFF);
  } else if ((data & 0xFF) >= 'A' && (data & 0xFF) <= 'Z') {
    return (char)(data & 0xFF);
  }
  return DEFAULT_CHAR;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  (void)pxTask;
  (void)pcTaskName;
  while (1) {
    __NOP();
  }
}

void vApplicationIdleHook(void) {
  while (1) {
    __NOP();
  }
}

static void keyboardTask(void *pvParameters) {
  (void)pvParameters;
  USB_HID_KeyboardReport_TypeDef report;
  uint8_t isKeyPressed = 0;
  uint8_t hidCode;

  hidCode = charToHIDCode(storedChar);

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
        report.Keycodes[0] = hidCode;
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

static void vEepromTask(void *pvParameters) {
  (void)pvParameters;
  uint8_t rxChar;

  while (1) {
    if (xQueueReceive(xCharQueue, &rxChar, portMAX_DELAY) == pdTRUE) {
      if (receivedFlag) {
        continue;
      }
      char c = (char)rxChar;
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        storedChar = (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
        receivedFlag = 1;

        uint32_t readVal = 0;
        taskENTER_CRITICAL();
        RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
        EEPROM_SetLatency(EEPROM_Latency_3);
        EEPROM_ErasePage(EEPROM_STORAGE_ADDR, EEPROM_Main_Bank_Select);
        EEPROM_ProgramWord(EEPROM_STORAGE_ADDR, EEPROM_Main_Bank_Select, (uint32_t)storedChar);
        EEPROM_UpdateDCache();
        readVal = EEPROM_ReadWord(EEPROM_STORAGE_ADDR, EEPROM_Main_Bank_Select);
        RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, DISABLE);
        taskEXIT_CRITICAL();

        if ((readVal & 0xFF) == storedChar) {
          const char *okMsg = "OK\r\n";
          while (USB_CDC_SendData((uint8_t *)okMsg, 4) != USB_SUCCESS) {
            vTaskDelay(pdMS_TO_TICKS(1));
          }
        } else {
          char errMsg[32];
          errMsg[0] = 'E';
          errMsg[1] = 'R';
          errMsg[2] = 'R';
          errMsg[3] = ':';
          errMsg[4] = ' ';
          errMsg[5] = (char)storedChar;
          errMsg[6] = '-';
          errMsg[7] = '>';
          errMsg[8] = (char)(readVal & 0xFF);
          uint32_t len = 11;
          if (readVal == 0xFFFFFFFF) {
            errMsg[9] = 'F';
            errMsg[10] = 'F';
            errMsg[11] = '\r';
            errMsg[12] = '\n';
            len = 13;
          } else {
            errMsg[9] = '\r';
            errMsg[10] = '\n';
            len = 11;
          }
          while (USB_CDC_SendData((uint8_t *)errMsg, len) != USB_SUCCESS) {
            vTaskDelay(pdMS_TO_TICKS(1));
          }
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
        NVIC_SystemReset();
      } else {
        const char *errMsg = "ERROR: Invalid character\r\n";
        while (USB_CDC_SendData((uint8_t *)errMsg, 26) != USB_SUCCESS) {
          vTaskDelay(pdMS_TO_TICKS(1));
        }
      }
    }
  }
}

static void initVcomMode(void) {
#ifdef USB_CDC_LINE_CODING_SUPPORTED
  LineCoding.dwDTERate = 115200;
  LineCoding.bCharFormat = 0;
  LineCoding.bParityType = 0;
  LineCoding.bDataBits = 8;
#endif

  xCharQueue = xQueueCreate(VCOM_BUFFER_LENGTH, sizeof(uint8_t));

  xTaskCreate(vEepromTask, "vEepromTask", configMINIMAL_STACK_SIZE * 2, NULL,
              tskIDLE_PRIORITY + 2, NULL);

  USB_CDC_Init(VcomBuffer, 1, SET);
  Board_USB_Init(true);
}

static void initKeyboardMode(void) {
  // storedChar = loadCharFromEEPROM();
  USB_HID_Init();
  Board_USB_Init(false);
  xTaskCreate(keyboardTask, "kbdTask", configMINIMAL_STACK_SIZE * 2, NULL,
              tskIDLE_PRIORITY + 2, NULL);
}

int main(void) {
  CLK_Init_80_mhz();
  Board_GPIO_Init();

  /* Button held at power-on → VCOM; released → HID keyboard */
  App_ComboSetCdcMode(Board_ButtonPressed());
  if (!App_ComboCdcMode()) {
    storedChar = loadCharFromEEPROM();
  }

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

USB_Result USB_CDC_RecieveData(uint8_t *Buffer, uint32_t Length) {
  if (!App_ComboCdcMode()) {
    return USB_ERROR;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  for (uint32_t i = 0; i < Length; i++) {
    xQueueSendFromISR(xCharQueue, &Buffer[i], &xHigherPriorityTaskWoken);
  }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

  USB_CDC_ReceiveStart();

  return USB_SUCCESS;
}

#ifdef USB_CDC_LINE_CODING_SUPPORTED
USB_Result USB_CDC_GetLineCoding(uint16_t wINDEX,
                                 USB_CDC_LineCoding_TypeDef *DATA) {
  if (!App_ComboCdcMode() || wINDEX != 0) {
    return USB_ERR_INV_REQ;
  }
  *DATA = LineCoding;
  return USB_SUCCESS;
}

USB_Result USB_CDC_SetLineCoding(uint16_t wINDEX,
                                 const USB_CDC_LineCoding_TypeDef *DATA) {
  if (!App_ComboCdcMode() || wINDEX != 0) {
    return USB_ERR_INV_REQ;
  }
  LineCoding = *DATA;
  return USB_SUCCESS;
}
#endif

/**
  ******************************************************************************
  * @file    K1986VE1xI_IT.h
  * @author  Milandr Application Team
  * @version V2.1.0i
  * @date    24/07/2024
  * @brief   This file contains all the functions prototypes
  *          for the interrupt handlers for MCU K1986VE1xI.
  ******************************************************************************
  * <br><br>
  *
  * THE PRESENT FIRMWARE IS FOR GUIDANCE ONLY. IT AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING MILANDR'S PRODUCTS IN ORDER TO FACILITATE
  * THE USE AND SAVE TIME. MILANDR SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES RESULTING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR A USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2025 Milandr</center></h2>
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __K1986VE1xI_IT_H
#define __K1986VE1xI_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

#if defined (USE_K1986VE1xI) && !defined (USE_MDR32F1QI)

/* Function prototypes -------------------------------------------------------*/
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void MIL_STD_1553B2_IRQHandler(void);
void MIL_STD_1553B1_IRQHandler(void);
void USB_IRQHandler(void);
void CAN1_IRQHandler(void);
void CAN2_IRQHandler(void);
void DMA_IRQHandler(void);
void UART1_IRQHandler(void);
void UART2_IRQHandler(void);
void SSP1_IRQHandler(void);
void BUSY_IRQHandler(void);
void ARINC429R_IRQHandler(void);
void POWER_IRQHandler(void);
void WWDG_IRQHandler(void);
void TIMER4_IRQHandler(void);
void TIMER1_IRQHandler(void);
void TIMER2_IRQHandler(void);
void TIMER3_IRQHandler(void);
void ADC_IRQHandler(void);
void ETHERNET_IRQHandler(void);
void SSP3_IRQHandler(void);
void SSP2_IRQHandler(void);
void ARINC429T1_IRQHandler(void);
void ARINC429T2_IRQHandler(void);
void ARINC429T3_IRQHandler(void);
void ARINC429T4_IRQHandler(void);
void BKP_IRQHandler(void);
void EXT_INT1_IRQHandler(void);
void EXT_INT2_IRQHandler(void);
void EXT_INT3_IRQHandler(void);
void EXT_INT4_IRQHandler(void);

#endif /* #if defined (USE_K1986VE1xI) && !defined (USE_MDR32F1QI) */

#ifdef __cplusplus
}
#endif

#endif /* __K1986VE1xI_IT_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE K1986VE1xI_IT.h */



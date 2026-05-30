/**
 * @file    dma_control_table.c
 * @brief   DMA control table for GCC (not provided in SPL dma.c for __GNUC__).
 */

#include "MDR32FxQI_dma.h"

#if (DMA_AlternateData == 1)
DMA_CtrlDataTypeDef DMA_ControlTable[(32U * DMA_AlternateData) + DMA_Channels_Number]
    __attribute__((aligned(1024)));
#elif (DMA_AlternateData == 0)
DMA_CtrlDataTypeDef DMA_ControlTable[DMA_Channels_Number] __attribute__((aligned(1024)));
#endif

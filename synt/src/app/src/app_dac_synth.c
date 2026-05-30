/**
 * @file    app_dac_synth.c
 * @brief   Sine wave output on DAC via DMA (based on Milandr DAC DMA_SineWave example).
 */

#include "app_dac_synth.h"
#include "board_pins.h"
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_dac.h"
#include "MDR32FxQI_timer.h"
#include "MDR32FxQI_dma.h"
#include "K1986VE9xI.h"

#define SYNTH_WAVE_SAMPLES 32U
#define SYNTH_MID_CODE     2047U
#define SYNTH_HZ_A4        440U
#define SYNTH_NOTE_A4      69U
#define SYNTH_NO_NOTE      0xFFU

static uint16_t Sine12bit[SYNTH_WAVE_SAMPLES] = {
    2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
    3939, 3750, 3498, 3185, 2831, 2447, 2047, 1647, 1263, 909,
    599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647
};

static volatile uint8_t synth_active_note = SYNTH_NO_NOTE;
static const uint32_t synth_dac_data_reg = (uint32_t)&(MDR_DAC->DAC2_DATA);

#if (BOARD_DAC_CHANNEL != 2)
#error "synt: only DAC2 pins are supported (e.g. PE0 = DAC2_OUT)"
#endif

static uint32_t midi_note_to_hz(uint8_t note)
{
    uint32_t freq = SYNTH_HZ_A4;
    int semitones;

    if (note >= SYNTH_NOTE_A4) {
        semitones = (int)note - (int)SYNTH_NOTE_A4;
        while (semitones-- > 0) {
            freq = (freq * 11849U + 2000U) / 10000U;
        }
    } else {
        semitones = (int)SYNTH_NOTE_A4 - (int)note;
        while (semitones-- > 0) {
            freq = (freq * 10000U + 5949U) / 11849U;
        }
    }

    return freq;
}

static void synth_set_timer_hz(uint32_t freq_hz)
{
    uint32_t period;

    if (freq_hz == 0U) {
        TIMER_Cmd(MDR_TIMER1, DISABLE);
        return;
    }

    period = SystemCoreClock / (freq_hz * SYNTH_WAVE_SAMPLES);
    if (period < 2U) {
        period = 2U;
    }
    if (period > 0xFFFFU) {
        period = 0xFFFFU;
    }

    TIMER_CntAutoreloadConfig(MDR_TIMER1, (uint16_t)(period - 1U),
                              TIMER_ARR_Update_Immediately);
    TIMER_SetCounter(MDR_TIMER1, 0U);
    TIMER_Cmd(MDR_TIMER1, ENABLE);
}

static void synth_dac_hold_mid(void)
{
    DAC2_SetData(SYNTH_MID_CODE);
}

void DMA_IRQHandler(void)
{
    if (DMA_GetFlagStatus(DMA_Channel_TIM1, DMA_FLAG_CHNL_ALT) == RESET) {
        DMA_ChannelReloadCycle(DMA_Channel_TIM1, DMA_CTRL_DATA_ALTERNATE,
                               SYNTH_WAVE_SAMPLES, DMA_Mode_PingPong);
    } else {
        DMA_ChannelReloadCycle(DMA_Channel_TIM1, DMA_CTRL_DATA_PRIMARY,
                               SYNTH_WAVE_SAMPLES, DMA_Mode_PingPong);
    }
}

void Synth_Init(void)
{
    PORT_InitTypeDef PORT_InitStructure;
    TIMER_CntInitTypeDef sTIM_CntInit;
    DMA_ChannelInitTypeDef DMA_InitStr;
    DMA_CtrlDataInitTypeDef DMA_PriCtrlStr;
    DMA_CtrlDataInitTypeDef DMA_AltCtrlStr;

    RST_CLK_PCLKcmd((RST_CLK_PCLK_DMA | BOARD_DAC_PCLK | RST_CLK_PCLK_TIMER1 |
                     RST_CLK_PCLK_DAC),
                    ENABLE);

    PORT_StructInit(&PORT_InitStructure);
    PORT_InitStructure.PORT_Pin = BOARD_DAC_BIT;
    PORT_InitStructure.PORT_OE = PORT_OE_OUT;
    PORT_InitStructure.PORT_MODE = PORT_MODE_ANALOG;
    PORT_Init(BOARD_DAC_PORT, &PORT_InitStructure);

    DMA_DeInit();

    DMA_PriCtrlStr.DMA_SourceBaseAddr = (uint32_t)Sine12bit;
    DMA_PriCtrlStr.DMA_DestBaseAddr = synth_dac_data_reg;
    DMA_PriCtrlStr.DMA_SourceIncSize = DMA_SourceIncHalfword;
    DMA_PriCtrlStr.DMA_DestIncSize = DMA_DestIncNo;
    DMA_PriCtrlStr.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_PriCtrlStr.DMA_Mode = DMA_Mode_PingPong;
    DMA_PriCtrlStr.DMA_CycleSize = SYNTH_WAVE_SAMPLES;
    DMA_PriCtrlStr.DMA_NumContinuous = DMA_Transfers_1;
    DMA_PriCtrlStr.DMA_SourceProtCtrl = DMA_SourcePrivileged;
    DMA_PriCtrlStr.DMA_DestProtCtrl = DMA_DestPrivileged;

    DMA_AltCtrlStr = DMA_PriCtrlStr;

    DMA_StructInit(&DMA_InitStr);
    DMA_InitStr.DMA_PriCtrlData = &DMA_PriCtrlStr;
    DMA_InitStr.DMA_AltCtrlData = &DMA_AltCtrlStr;
    DMA_InitStr.DMA_Priority = DMA_Priority_Default;
    DMA_InitStr.DMA_UseBurst = DMA_BurstClear;
    DMA_InitStr.DMA_SelectDataStructure = DMA_CTRL_DATA_PRIMARY;
    DMA_Init(DMA_Channel_TIM1, &DMA_InitStr);
    DMA_Cmd(DMA_Channel_TIM1, ENABLE);

    DAC_DeInit();
    DAC2_Init(DAC2_AVCC);
    DAC2_Cmd(ENABLE);
    synth_dac_hold_mid();

    TIMER_DeInit(MDR_TIMER1);
    TIMER_BRGInit(MDR_TIMER1, TIMER_HCLKdiv1);
    sTIM_CntInit.TIMER_Prescaler = 0;
    sTIM_CntInit.TIMER_Period = 0xFF;
    sTIM_CntInit.TIMER_CounterMode = TIMER_CntMode_ClkFixedDir;
    sTIM_CntInit.TIMER_CounterDirection = TIMER_CntDir_Up;
    sTIM_CntInit.TIMER_EventSource = TIMER_EvSrc_TIM_CLK;
    sTIM_CntInit.TIMER_FilterSampling = TIMER_FDTS_TIMER_CLK_div_1;
    sTIM_CntInit.TIMER_ARR_UpdateMode = TIMER_ARR_Update_Immediately;
    sTIM_CntInit.TIMER_ETR_FilterConf = TIMER_Filter_1FF_at_TIMER_CLK;
    sTIM_CntInit.TIMER_ETR_Prescaler = TIMER_ETR_Prescaler_None;
    sTIM_CntInit.TIMER_ETR_Polarity = TIMER_ETRPolarity_NonInverted;
    sTIM_CntInit.TIMER_BRK_Polarity = TIMER_BRKPolarity_NonInverted;
    TIMER_CntInit(MDR_TIMER1, &sTIM_CntInit);
    TIMER_DMACmd(MDR_TIMER1, TIMER_STATUS_CNT_ARR, ENABLE);

    NVIC_EnableIRQ(DMA_IRQn);
}

void Synth_NoteOn(uint8_t note)
{
    synth_active_note = note;
    synth_set_timer_hz(midi_note_to_hz(note));
}

void Synth_NoteOff(uint8_t note)
{
    if (synth_active_note != note) {
        return;
    }

    Synth_AllNotesOff();
}

void Synth_AllNotesOff(void)
{
    synth_active_note = SYNTH_NO_NOTE;
    TIMER_Cmd(MDR_TIMER1, DISABLE);
    synth_dac_hold_mid();
}

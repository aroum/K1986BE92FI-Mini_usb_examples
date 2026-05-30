/**
 * @file    app_dac_synth.h
 * @brief   DAC sine-wave synthesizer (DMA + TIMER1), driven by MIDI note number.
 */

#ifndef APP_DAC_SYNTH_H
#define APP_DAC_SYNTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void Synth_Init(void);
void Synth_NoteOn(uint8_t note);
void Synth_NoteOff(uint8_t note);
void Synth_AllNotesOff(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_DAC_SYNTH_H */

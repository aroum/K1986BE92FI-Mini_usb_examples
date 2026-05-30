/**
 * @file    app.c
 * @brief   USB MIDI synthesizer: incoming notes drive DAC sine output.
 */

#include "app.h"
#include "board_gpio.h"
#include "board_usb.h"
#include "app_usb_midi.h"
#include "app_dac_synth.h"
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

static void on_usb_midi_packet(const USB_MIDI_Packet_TypeDef *packet)
{
    uint8_t cin = packet->CableCIN & 0x0FU;

    if (cin == MIDI_CIN_NOTE_ON) {
        if (packet->MIDIData2 > 0U) {
            Synth_NoteOn(packet->MIDIData1);
            Board_LED_On();
        } else {
            Synth_NoteOff(packet->MIDIData1);
            Board_LED_Off();
        }
    } else if (cin == MIDI_CIN_NOTE_OFF) {
        Synth_NoteOff(packet->MIDIData1);
        Board_LED_Off();
    }
}

int main(void)
{
    CLK_Init_80_mhz();
    Board_GPIO_Init();
    Synth_Init();
    USB_MIDI_Init();
    USB_MIDI_SetRxHandler(on_usb_midi_packet);
    Board_USB_Init(true);
    vTaskStartScheduler();
    while (1) {
        __NOP();
    }
    return 0;
}

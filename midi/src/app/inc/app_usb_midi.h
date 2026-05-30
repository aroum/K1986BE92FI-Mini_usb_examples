/**
 * @file    app_usb_midi.h
 * @brief   USB MIDI device driver definitions and API.
 */

#ifndef __APP_USB_MIDI_H
#define __APP_USB_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MDR32FxQI_config.h"
#include "MDR32FxQI_usb_device.h"

/* USB MIDI Event (CIN) codes for 3-byte channel voice messages */
#define MIDI_CIN_NOTE_OFF                0x08
#define MIDI_CIN_NOTE_ON                 0x09

/* Bulk IN (0x81) on EP1, bulk OUT (0x03) on EP3 — same EP number cannot be IN+OUT on MDR32 */
#define USB_MIDI_EP_SEND                 USB_EP1
#define USB_MIDI_EP_RECEIVE              USB_EP3

#pragma pack(push, 1)
/**
 * @brief USB MIDI 1.0 event packet (4 bytes).
 */
typedef struct
{
    uint8_t CableCIN;
    uint8_t MIDIStatus;
    uint8_t MIDIData1;
    uint8_t MIDIData2;
} USB_MIDI_Packet_TypeDef;
#pragma pack(pop)

USB_Result USB_MIDI_Init(void);
USB_Result USB_MIDI_Reset(void);
USB_Result USB_MIDI_ConfigureEndpoints(void);
USB_Result USB_MIDI_SendPacket(const USB_MIDI_Packet_TypeDef *packet);
USB_Result USB_MIDI_SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
USB_Result USB_MIDI_SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
USB_Result USB_MIDI_GetDescriptor(uint16_t wVALUE, uint16_t wINDEX, uint16_t wLENGTH);
USB_Result USB_MIDI_ClassRequest(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_USB_MIDI_H */

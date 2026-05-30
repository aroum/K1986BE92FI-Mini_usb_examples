/**
 * @file    app_usb_midi.c
 * @brief   USB MIDI device: receive Note On/Off, forward to application handler.
 */

#include "app_usb_midi.h"
#include "K1986VE9xI.h"

#define USB_MIDI_RX_MIN_LENGTH 1U

static USB_MIDI_RxHandler usb_midi_rx_handler;
static uint8_t USB_MIDI_RxBuffer[64];

static USB_Result USB_MIDI_OnDataOut(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length);
static USB_Result USB_MIDI_SendError(USB_EP_TypeDef EPx, uint32_t STS, uint32_t TS, uint32_t CTRL);

static void USB_MIDI_DispatchRx(const uint8_t *buffer, uint32_t length)
{
    uint32_t offset;

    if (usb_midi_rx_handler == 0) {
        return;
    }

    for (offset = 0; offset + 3U < length; offset += 4U) {
        USB_MIDI_Packet_TypeDef packet;

        packet.CableCIN = buffer[offset];
        packet.MIDIStatus = buffer[offset + 1U];
        packet.MIDIData1 = buffer[offset + 2U];
        packet.MIDIData2 = buffer[offset + 3U];
        usb_midi_rx_handler(&packet);
    }
}

/* ---------------------- USB MIDI Descriptors ------------------------------ */

static const uint8_t Usb_MIDI_Device_Descriptor[18] = {
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 64,
    0x83, 0x04, 0x13, 0x57, 0x00, 0x01, 0x01, 0x02, 0x03, 0x01
};

static const uint8_t Usb_MIDI_Configuration_Descriptor[101] = {
    0x09, 0x02, 0x65, 0x00, 0x02, 0x01, 0x00, 0x80, 50,
    0x09, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
    0x09, 0x24, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x01,
    0x09, 0x04, 0x01, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00,
    0x07, 0x24, 0x01, 0x00, 0x01, 0x41, 0x00,
    0x06, 0x24, 0x02, 0x01, 0x01, 0x00,
    0x06, 0x24, 0x02, 0x02, 0x02, 0x00,
    0x09, 0x24, 0x03, 0x01, 0x03, 0x01, 0x02, 0x01, 0x00,
    0x09, 0x24, 0x03, 0x02, 0x04, 0x01, 0x01, 0x01, 0x00,
    0x09, 0x05, 0x03, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x25, 0x01, 0x01, 0x01,
    0x09, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x05, 0x25, 0x01, 0x01, 0x03
};

static const uint8_t Usb_MIDI_String_LangID[4] = {0x04, 0x03, 0x09, 0x04};

static const uint8_t Usb_MIDI_String_Manuf[16] = {
    16, 0x03, 'M', 0, 'i', 0, 'l', 0, 'a', 0, 'n', 0, 'd', 0, 'r', 0
};

static const uint8_t Usb_MIDI_String_Prod[18] = {
    18, 0x03, 'U', 0, 'S', 0, 'B', 0, ' ', 0, 'S', 0, 'y', 0, 'n', 0, 't', 0
};

static const uint8_t Usb_MIDI_String_Serial[18] = {
    18, 0x03, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0
};

static USB_Result USB_MIDI_OnDataOut(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length)
{
    (void)EPx;

    USB_MIDI_DispatchRx(Buffer, Length);

    return USB_EP_doDataOut(USB_MIDI_EP_RECEIVE, USB_MIDI_RxBuffer,
                            USB_MIDI_RX_MIN_LENGTH, USB_MIDI_OnDataOut);
}

static USB_Result USB_MIDI_SendError(USB_EP_TypeDef EPx, uint32_t STS, uint32_t TS, uint32_t CTRL)
{
    (void)EPx;
    (void)STS;
    (void)TS;
    (void)CTRL;
    return USB_SUCCESS;
}

USB_Result USB_MIDI_Init(void)
{
    usb_midi_rx_handler = 0;
    return USB_SUCCESS;
}

USB_Result USB_MIDI_SetRxHandler(USB_MIDI_RxHandler handler)
{
    usb_midi_rx_handler = handler;
    return USB_SUCCESS;
}

USB_Result USB_MIDI_ConfigureEndpoints(void)
{
    USB_EP_Init(USB_MIDI_EP_SEND, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1,
                USB_MIDI_SendError);
    USB_EP_Init(USB_MIDI_EP_RECEIVE, USB_SEPx_CTRL_EPEN_Enable, 0);
    USB_EP_doDataOut(USB_MIDI_EP_RECEIVE, USB_MIDI_RxBuffer, USB_MIDI_RX_MIN_LENGTH,
                     USB_MIDI_OnDataOut);
    return USB_SUCCESS;
}

USB_Result USB_MIDI_Reset(void)
{
    USB_Result result = USB_DeviceReset();

    if (result == USB_SUCCESS) {
        result = USB_MIDI_ConfigureEndpoints();
    }

    return result;
}

USB_Result USB_MIDI_GetDescriptor(uint16_t wVALUE, uint16_t wINDEX, uint16_t wLENGTH)
{
    const uint8_t *pDescr = 0;
    uint32_t length = 0;
    USB_Result result = USB_SUCCESS;
    uint8_t descType = (uint8_t)(wVALUE >> 8);
    uint8_t descIndex = (uint8_t)(wVALUE & 0xFF);

    (void)wINDEX;

    switch (descType) {
    case USB_DEVICE:
        pDescr = Usb_MIDI_Device_Descriptor;
        length = sizeof(Usb_MIDI_Device_Descriptor);
        break;
    case USB_CONFIGURATION:
        pDescr = Usb_MIDI_Configuration_Descriptor;
        length = sizeof(Usb_MIDI_Configuration_Descriptor);
        break;
    case USB_STRING:
        switch (descIndex) {
        case 0:
            pDescr = Usb_MIDI_String_LangID;
            length = sizeof(Usb_MIDI_String_LangID);
            break;
        case 1:
            pDescr = Usb_MIDI_String_Manuf;
            length = sizeof(Usb_MIDI_String_Manuf);
            break;
        case 2:
            pDescr = Usb_MIDI_String_Prod;
            length = sizeof(Usb_MIDI_String_Prod);
            break;
        case 3:
            pDescr = Usb_MIDI_String_Serial;
            length = sizeof(Usb_MIDI_String_Serial);
            break;
        default:
            result = USB_ERROR;
            break;
        }
        break;
    default:
        result = USB_ERROR;
        break;
    }

    if (result == USB_SUCCESS && pDescr != 0) {
        if (length > wLENGTH) {
            length = wLENGTH;
        }
        result = USB_EP_doDataIn(USB_EP0, (uint8_t *)pDescr, length, USB_DeviceDoStatusOutAck);
    }

    return result;
}

USB_Result USB_MIDI_ClassRequest(void)
{
    USB_Result result = USB_SUCCESS;
    uint16_t wLength = USB_CurrentSetupPacket.wLength;
    uint8_t isDeviceToHost = (USB_CurrentSetupPacket.mRequestTypeData & 0x80U) != 0U;

    if (wLength > 0U) {
        static uint8_t classData[32];

        if (isDeviceToHost) {
            if (wLength > sizeof(classData)) {
                wLength = sizeof(classData);
            }
            result = USB_EP_doDataIn(USB_EP0, classData, wLength, USB_DeviceDoStatusOutAck);
        } else {
            if (wLength > sizeof(classData)) {
                wLength = sizeof(classData);
            }
            result = USB_EP_doDataOut(USB_EP0, classData, wLength, USB_DeviceDoStatusOutAck);
        }
    } else if (result == USB_SUCCESS) {
        result = isDeviceToHost ? USB_EP_doDataOut(USB_EP0, 0, 0, 0)
                                : USB_EP_doDataIn(USB_EP0, 0, 0, 0);
    }

    return result;
}

/**
 * @file    app_usb_midi.c
 * @brief   USB MIDI device driver implementation using Milandr SPL.
 */

#include "app_usb_midi.h"
#include "K1986VE9xI.h"

static volatile USB_Result USB_MIDI_SendDataStatus = USB_SUCCESS;
static USB_MIDI_Packet_TypeDef USB_MIDI_TxPacket;
static uint8_t USB_MIDI_RxBuffer[64];

static USB_Result USB_MIDI_OnDataOut(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length);
static USB_Result USB_MIDI_SendError(USB_EP_TypeDef EPx, uint32_t STS, uint32_t TS, uint32_t CTRL);

/* ---------------------- USB MIDI Descriptors ------------------------------ */

static const uint8_t Usb_MIDI_Device_Descriptor[18] =
{
    0x12,
    0x01,
    0x00, 0x02,
    0x00,
    0x00,
    0x00,
    64,
    0x83, 0x04,
    0x13, 0x57,
    0x00, 0x01,
    0x01,
    0x02,
    0x03,
    0x01
};

/* Configuration + Audio Control + MIDI Streaming (USB MIDI 1.0) */
static const uint8_t Usb_MIDI_Configuration_Descriptor[101] =
{
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

static const uint8_t Usb_MIDI_String_LangID[4] =
{
    0x04, 0x03, 0x09, 0x04
};

static const uint8_t Usb_MIDI_String_Manuf[16] =
{
    16, 0x03,
    'M', 0, 'i', 0, 'l', 0, 'a', 0, 'n', 0, 'd', 0, 'r', 0
};

static const uint8_t Usb_MIDI_String_Prod[22] =
{
    22, 0x03,
    'U', 0, 'S', 0, 'B', 0, ' ', 0, 'M', 0, 'I', 0, 'D', 0, 'I', 0
};

static const uint8_t Usb_MIDI_String_Serial[18] =
{
    18, 0x03,
    '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0
};

/* ---------------------- Private Callbacks --------------------------------- */

static USB_Result USB_MIDI_OnDataSent(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length)
{
    (void)EPx;
    (void)Buffer;
    (void)Length;

    USB_MIDI_SendDataStatus = USB_SUCCESS;
    return USB_SUCCESS;
}

static USB_Result USB_MIDI_OnDataOut(USB_EP_TypeDef EPx, uint8_t *Buffer, uint32_t Length)
{
    (void)EPx;
    (void)Buffer;
    (void)Length;

    return USB_EP_doDataOut(USB_MIDI_EP_RECEIVE, USB_MIDI_RxBuffer,
                            sizeof(USB_MIDI_RxBuffer), USB_MIDI_OnDataOut);
}

static USB_Result USB_MIDI_SendError(USB_EP_TypeDef EPx, uint32_t STS, uint32_t TS, uint32_t CTRL)
{
    (void)EPx;
    (void)STS;
    (void)TS;
    (void)CTRL;

    USB_MIDI_SendDataStatus = USB_SUCCESS;
    return USB_SUCCESS;
}

/* ---------------------- Public API ---------------------------------------- */

USB_Result USB_MIDI_Init(void)
{
    USB_MIDI_SendDataStatus = USB_SUCCESS;
    return USB_SUCCESS;
}

USB_Result USB_MIDI_ConfigureEndpoints(void)
{
    USB_EP_Init(USB_MIDI_EP_SEND, USB_SEPx_CTRL_EPEN_Enable | USB_SEPx_CTRL_EPDATASEQ_Data1,
                USB_MIDI_SendError);
    USB_EP_Init(USB_MIDI_EP_RECEIVE, USB_SEPx_CTRL_EPEN_Enable, 0);
    USB_EP_doDataOut(USB_MIDI_EP_RECEIVE, USB_MIDI_RxBuffer,
                     sizeof(USB_MIDI_RxBuffer), USB_MIDI_OnDataOut);
    USB_MIDI_SendDataStatus = USB_SUCCESS;

    return USB_SUCCESS;
}

USB_Result USB_MIDI_Reset(void)
{
    USB_Result result;

    result = USB_DeviceReset();

    if (result == USB_SUCCESS)
    {
        result = USB_MIDI_ConfigureEndpoints();
    }

    return result;
}

USB_Result USB_MIDI_SendPacket(const USB_MIDI_Packet_TypeDef *packet)
{
    USB_Result result;

    if (USB_DeviceContext.USB_DeviceState != USB_DEV_STATE_CONFIGURED)
    {
        return USB_ERR_BUSY;
    }

    if (USB_MIDI_SendDataStatus != USB_SUCCESS)
    {
        return USB_ERR_BUSY;
    }

    USB_MIDI_TxPacket = *packet;
    USB_MIDI_SendDataStatus = USB_ERR_BUSY;

#ifdef USB_INT_HANDLE_REQUIRED
    NVIC_DisableIRQ(USB_IRQn);
#endif
    result = USB_EP_doDataIn(USB_MIDI_EP_SEND, (uint8_t *)&USB_MIDI_TxPacket,
                             sizeof(USB_MIDI_TxPacket), USB_MIDI_OnDataSent);
#ifdef USB_INT_HANDLE_REQUIRED
    NVIC_EnableIRQ(USB_IRQn);
#endif

    if (result != USB_SUCCESS)
    {
        USB_MIDI_SendDataStatus = USB_SUCCESS;
    }

    return result;
}

USB_Result USB_MIDI_SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    USB_MIDI_Packet_TypeDef packet;

    packet.CableCIN = MIDI_CIN_NOTE_ON;
    packet.MIDIStatus = (uint8_t)(0x90 | (channel & 0x0F));
    packet.MIDIData1 = (uint8_t)(note & 0x7F);
    packet.MIDIData2 = (uint8_t)(velocity & 0x7F);

    return USB_MIDI_SendPacket(&packet);
}

USB_Result USB_MIDI_SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    USB_MIDI_Packet_TypeDef packet;

    packet.CableCIN = MIDI_CIN_NOTE_OFF;
    packet.MIDIStatus = (uint8_t)(0x80 | (channel & 0x0F));
    packet.MIDIData1 = (uint8_t)(note & 0x7F);
    packet.MIDIData2 = (uint8_t)(velocity & 0x7F);

    return USB_MIDI_SendPacket(&packet);
}

USB_Result USB_MIDI_GetDescriptor(uint16_t wVALUE, uint16_t wINDEX, uint16_t wLENGTH)
{
    const uint8_t *pDescr = 0;
    uint32_t length = 0;
    USB_Result result = USB_SUCCESS;
    uint8_t descType = (uint8_t)(wVALUE >> 8);
    uint8_t descIndex = (uint8_t)(wVALUE & 0xFF);

    (void)wINDEX;

    switch (descType)
    {
        case USB_DEVICE:
            pDescr = Usb_MIDI_Device_Descriptor;
            length = sizeof(Usb_MIDI_Device_Descriptor);
            break;

        case USB_CONFIGURATION:
            pDescr = Usb_MIDI_Configuration_Descriptor;
            length = sizeof(Usb_MIDI_Configuration_Descriptor);
            break;

        case USB_STRING:
            switch (descIndex)
            {
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

    if (result == USB_SUCCESS && pDescr != 0)
    {
        if (length > wLENGTH)
        {
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
    uint8_t isDeviceToHost = (USB_CurrentSetupPacket.mRequestTypeData & 0x80) != 0;

    if (wLength > 0)
    {
        static uint8_t classData[32];

        if (isDeviceToHost)
        {
            if (wLength > sizeof(classData))
            {
                wLength = sizeof(classData);
            }
            result = USB_EP_doDataIn(USB_EP0, classData, wLength, USB_DeviceDoStatusOutAck);
        }
        else
        {
            if (wLength > sizeof(classData))
            {
                wLength = sizeof(classData);
            }
            result = USB_EP_doDataOut(USB_EP0, classData, wLength, USB_DeviceDoStatusOutAck);
        }
    }
    else if (result == USB_SUCCESS)
    {
        result = isDeviceToHost ?
                 USB_EP_doDataOut(USB_EP0, 0, 0, 0) :
                 USB_EP_doDataIn(USB_EP0, 0, 0, 0);
    }

    return result;
}

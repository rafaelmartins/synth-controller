#include <tusb.h>
#include <controller.h>

static uint8_t buf[1024];

static const tusb_desc_device_t desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0110,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0x16c0, // free from v-usb
    .idProduct          = 0x05dc, // free from v-usb
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

const uint8_t*
tud_descriptor_device_cb(void)
{
    return (const uint8_t*) &desc_device;
}


static const uint8_t descriptor_conf[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_MIDI_DESCRIPTOR(0, 0, 0x01, TUSB_DIR_IN_MASK | 0x01, 64),
};

const uint8_t*
tud_descriptor_configuration_cb(uint8_t index)
{
    return descriptor_conf;
}


#define _DESCRIPTION_HEADER(len) (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * (len) + 2))
static const uint16_t lang[] = {_DESCRIPTION_HEADER(1), 0x0409};
static const uint16_t manufacturer[] = {_DESCRIPTION_HEADER(6), 'r', 'g', 'm', '.', 'i', 'o'};
static const uint16_t product[] = {_DESCRIPTION_HEADER(16), 's', 'y', 'n', 't', 'h', '-', 'c', 'o', 'n', 't', 'r', 'o', 'l', 'l', 'e', 'r'};
static const uint16_t serial[] = {_DESCRIPTION_HEADER(6), '1', '2', '3', '4', '5', '6'};
#undef _DESCRIPTION_HEADER

const uint16_t*
tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    switch (index) {
        case 0:
            return lang;
        case 1:
            return manufacturer;
        case 2:
            return product;
        case 3:
            return serial;
    }

    return NULL;
}


bool
tud_vendor_control_request_cb(uint8_t rhport, tusb_control_request_t const * request)
{
    uint16_t l = controller_usb_request_cb(
        request->bRequest,
        request->bmRequestType_bit.direction == TUSB_DIR_OUT,
        request->wValue,
        request->wIndex,
        buf);
    return tud_control_xfer(rhport, request, buf, l);
}


bool
tud_vendor_control_complete_cb(uint8_t rhport, tusb_control_request_t const * request)
{
    uint16_t len = request->wLength;
    controller_usb_data_cb(
        request->bRequest,
        buf,
        len > 1024 ? 1024 : len);
    // we assume that len will never be > 1024
    return true;
}

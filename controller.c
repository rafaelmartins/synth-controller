#include <string.h>
#include <hardware/gpio.h>
#include <pico/assert.h>
#include <tusb.h>
#include <cv-note.h>
#include <ssd1306.h>
#include <controller.h>

#define assert_channel(c) hard_assert(c >= CHANNEL_1 && c <= CHANNEL_2)

typedef enum {
    DUP_STRATEGY_LAST,
    DUP_STRATEGY_HIGHER,
    DUP_STRATEGY_LOWER,
} dup_strategy_t;

typedef enum {
    CHANNEL_1,
    CHANNEL_2,
    _NUM_CHANNELS,
} channel_t;

struct channel {
    cv_note_t *cv;
    uint gate;
    dup_strategy_t dup_strategy;
    uint8_t midi_channel;
    uint8_t note;
    bool playing;
    char str[22];
};

static struct channel channels[_NUM_CHANNELS];
static char midi_buf[64];
static ssd1306_t *display = NULL;


static void
display_render(void)
{
    hard_assert(display);

    ssd1306_clear(display);
    ssd1306_add_string_line(display, 0, "Synth Controller",
        SSD1306_LINE_ALIGN_CENTER);
    for (size_t i = 0; i < _NUM_CHANNELS; i++) {
        cv_note_to_string(channels[i].str + 16, sizeof(channels[i].str) - 16,
            channels[i].note);
        ssd1306_add_string_line(display, 3 + (2 * i), channels[i].str,
            SSD1306_LINE_ALIGN_LEFT);
    }
    ssd1306_render(display);
}


static void
set_midi_channel(channel_t c, uint8_t midi_channel)
{
    channels[c].midi_channel = midi_channel;
    char *str = channels[c].str + 10;

    if (midi_channel == 0xff) {
        *str++ = ' ';
        *str++ = ' ';
        *str++ = ' ';
        *str++ = ' ';
    }
    else {
        *str++ = '(';
        *str++ = (midi_channel / 10) + '0';
        *str++ = (midi_channel % 10) + '0';
        *str++ = ')';
    }
}


void
controller_init(ssd1306_t *disp, cv_note_t *cv1, uint gate1, cv_note_t *cv2, uint gate2)
{
    hard_assert(!display);

    display = disp;

    channels[0].cv = cv1;
    channels[0].gate = gate1;
    channels[1].cv = cv2;
    channels[1].gate = gate2;

    for (size_t i = 0; i < _NUM_CHANNELS; i++) {
        channels[i].dup_strategy = DUP_STRATEGY_LAST;
        channels[i].note = 0;
        channels[i].playing = false;
        strcpy(channels[i].str, "Channel       : ");
        channels[i].str[8] = '1' + i;
        set_midi_channel(i, i);

        gpio_init(channels[i].gate);
        gpio_pull_down(channels[i].gate);
        gpio_set_dir(channels[i].gate, GPIO_OUT);
    }

    display_render();
}


static bool
note_on(channel_t c, uint8_t note)
{
    assert_channel(c);

    if (channels[c].playing) {
        switch (channels[c].dup_strategy) {
            case DUP_STRATEGY_LAST:
                break;
            case DUP_STRATEGY_HIGHER:
                if (channels[c].note >= note)
                    return true;
            case DUP_STRATEGY_LOWER:
                if (channels[c].note <= note)
                    return true;
        }
    }

    gpio_put(channels[c].gate, true);
    channels[c].playing = cv_note_set(channels[c].cv, note);
    if (!channels[c].playing) {
        gpio_put(channels[c].gate, false);
    }
    else {
        channels[c].note = note;
        display_render();
    }
    return channels[c].playing;
}


static bool
note_off(channel_t c)
{
    assert_channel(c);

    if (!channels[c].playing)
        return true;

    gpio_put(channels[c].gate, false);
    channels[c].playing = !cv_note_unset(channels[c].cv);
    channels[c].note = 0;
    display_render();
    return !channels[c].playing;
}


void
controller_midi_task(void)
{
    if (!tud_midi_available())
        return;

    uint32_t count = tud_midi_read(midi_buf, sizeof(midi_buf));

    bool off = (count >= 2) && ((midi_buf[0] >> 4) == 0x8);
    bool on  = (count >= 3) && ((midi_buf[0] >> 4) == 0x9);

    if ((!on) && (!off))
        return;

    uint8_t chan = midi_buf[0] & 0x0f;
    for (size_t i = 0; i < _NUM_CHANNELS; i++) {
        if (channels[i].midi_channel != 0xff && channels[i].midi_channel == chan) {
            if (off || (on && midi_buf[2] == 0))
                note_off(i);
            else if (on)
                note_on(i, midi_buf[1]);
        }
    }
}


typedef enum {
    CMD_MIDI_CHANNEL = 1,
    CMD_DUP_STRATEGY,
} usb_command_t;

uint16_t
controller_usb_request_cb(uint8_t cmd, bool write, uint16_t val, uint16_t idx, uint8_t *buf)
{
    switch ((usb_command_t) cmd) {
        case CMD_MIDI_CHANNEL: {
            if (idx < CHANNEL_1 || idx > CHANNEL_2)
                return 0;

            if (!write) {
                buf[0] = channels[idx].midi_channel;
                return 1;
            }

            set_midi_channel(idx, val);
            display_render();
            break;
        }

        case CMD_DUP_STRATEGY: {
            if (idx < CHANNEL_1 || idx > CHANNEL_2)
                return 0;

            if (val < DUP_STRATEGY_LAST || val > DUP_STRATEGY_LOWER)
                return 0;

            if (!write) {
                buf[0] = channels[idx].dup_strategy;
                return 1;
            }

            channels[idx].dup_strategy = val;
            // FIXME: add this info to display
            break;
        }
    }

    return 0;
}


void
controller_usb_data_cb(uint8_t cmd, uint8_t *buf, uint16_t len)
{
}

#include <stdlib.h>
#include <string.h>
#include <pico/assert.h>
#include <mcp4822.h>
#include <cv-note.h>

// this is a 0-10v voltage control module built on top of a MCP4822 12-bit DAC.
// it is supposed to be used with 1V/oct VCOs, and converts MIDI note numbers
// in the A-1:G9 (9:127) range into a proportional voltage. the code here just
// sets the voltage, won't take care of note duration, gate setup, etc.


// - calculate output voltage according to played note:
//
// the MCP4822 actually goes up to 0xfff, as it is a 12-bit DAC, but we limit
// to 0xff0, because for 10 octaves we have 120 notes, and with this limit we
// get exact 0x22 steps for each note, improving precision.
//
// 9   -> A-1  -> 0
// 10  -> A#-1 -> 0x22
// 11  -> B#-1 -> 0x44
// ...
// 21  -> A0   -> 0x198
// 22  -> A#0  -> 0x1ba
// 23  -> B0   -> 0x1dc
// ...
// ...
// 125 -> F9   -> 0xf68
// 126 -> F#9  -> 0xf8a
// 127 -> G9   -> 0xfac
//
// the reference voltage from MCP4822 is 2.048V. we amplify it to 0-10V range
// using a non-inverted opamp-based amplifier, that must be calibrated to
// produce 10V when DAC output is 2.04V. the opamp must be powered with 12V,
// preferably from the synth power supply itself.

static uint16_t
to_mcp4822(uint8_t note)
{
    return note < 9 || note > 127 ? 0xf000 : (note - 9) * 0x22;
}


bool
cv_note_set(cv_note_t *c, uint8_t note)
{
    hard_assert(c);
    hard_assert(c->ddac);

    uint16_t n = to_mcp4822(note);
    if (n >> 12)
        return false;

    return mcp4822_set(c->ddac, c->dac, n);
}


bool
cv_note_unset(cv_note_t *c)
{
    hard_assert(c);
    hard_assert(c->ddac);

    return mcp4822_unset(c->ddac, c->dac);
}


static const char* note_names[] = {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B",
};

void
cv_note_to_string(char *str, size_t n, uint8_t note)
{
    hard_assert(str);
    hard_assert(n >= 5);

    if (note < 9 || note > 127) {
        *str = 0;
        return;
    }

    const char *name = note_names[note % 12];
    int oct = (note / 12) - 1;

    strncpy(str, name, 2);
    size_t l = strlen(name);
    if (oct < 0)
        str[l++] = '-';

    str[l++] = '0' + abs(oct);
    str[l] = 0;
}

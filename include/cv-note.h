#pragma once

#include <mcp4822.h>

typedef struct {
    mcp4822_t *ddac;
    mcp4822_dac_t dac;
} cv_note_t;

bool cv_note_set(cv_note_t *c, uint8_t note);
bool cv_note_unset(cv_note_t *c);
void cv_note_to_string(char *str, size_t n, uint8_t note);

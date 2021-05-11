#pragma once

#include <ssd1306.h>
#include <cv-note.h>

void controller_init(ssd1306_t *disp, cv_note_t *cv1, uint gate1,
    cv_note_t *cv2, uint gate2);
void controller_midi_task(void);
uint16_t controller_usb_request_cb(uint8_t cmd, bool write, uint16_t val,
    uint16_t idx, uint8_t *buf);
void controller_usb_data_cb(uint8_t cmd, uint8_t *buf, uint16_t len);

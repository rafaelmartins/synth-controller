#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <tusb.h>
#include <controller.h>
#include <cv-note.h>
#include <mcp4822.h>
#include <ssd1306.h>

static mcp4822_t ddac = {
    .spi = spi0,
    .cs = 5,
};

static ssd1306_t display = {
    .i2c = i2c0,
};

static cv_note_t cv1 = {
    .ddac = &ddac,
    .dac = MCP4822_DAC_A,
};

static cv_note_t cv2 = {
    .ddac = &ddac,
    .dac = MCP4822_DAC_B,
};


int
main() {
    stdio_init_all();
    tusb_init();

    i2c_init(i2c0, 200000);
    gpio_set_function(20, GPIO_FUNC_I2C);
    gpio_set_function(21, GPIO_FUNC_I2C);
    gpio_pull_up(20);
    gpio_pull_up(21);

    spi_init(spi0, 20000000);
    gpio_set_function(2, GPIO_FUNC_SPI);
    gpio_set_function(3, GPIO_FUNC_SPI);
    gpio_set_function(4, GPIO_FUNC_SPI);

    mcp4822_init(&ddac);
    ssd1306_init(&display);
    controller_init(&display, &cv1, 6, &cv2, 7);

    while (1) {
        tud_task();
        controller_midi_task();
    }
}

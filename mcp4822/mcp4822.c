#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/assert.h>
#include <pico/stdlib.h>
#include <mcp4822.h>


void
mcp4822_init(mcp4822_t *m)
{
    hard_assert(m);
    hard_assert(GPIO_FUNC_SPI != gpio_get_function(m->cs));

    gpio_init(m->cs);
    gpio_pull_up(m->cs);
    gpio_set_dir(m->cs, GPIO_OUT);
}


static bool
__not_in_flash_func(_set)(mcp4822_t *m, mcp4822_dac_t dac, bool shdn, uint16_t data)
{
    hard_assert(m);
    hard_assert(m->spi);

    // as the pico operates on 3v3, we can't enable 2x gain.
    uint8_t cmd[] = {
        // !A/B          | !GA      | !SHDN    | D11:D8
        ((dac & 1) << 7) | (1 << 5) | (((!shdn) & 1) << 4) | ((data >> 8) & 0xf),
        // D7:D0
        data & 0xff,
    };

    gpio_put(m->cs, false);
    int rv = spi_write_blocking(m->spi, cmd, 2);
    gpio_put(m->cs, true);

    return rv == 2;
}


bool
mcp4822_set(mcp4822_t *m, mcp4822_dac_t dac, uint16_t data)
{
    return _set(m, dac, false, data);
}


bool
mcp4822_unset(mcp4822_t *m, mcp4822_dac_t dac)
{
    return _set(m, dac, true, 0);
}

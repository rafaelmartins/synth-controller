#pragma once

#include <hardware/spi.h>

typedef struct {
    spi_inst_t *spi;
    uint cs;
} mcp4822_t;

typedef enum {
    MCP4822_DAC_A,
    MCP4822_DAC_B,
} mcp4822_dac_t;

void mcp4822_init(mcp4822_t *m);
bool mcp4822_set(mcp4822_t *m, mcp4822_dac_t dac, uint16_t data);
bool mcp4822_unset(mcp4822_t *m, mcp4822_dac_t dac);

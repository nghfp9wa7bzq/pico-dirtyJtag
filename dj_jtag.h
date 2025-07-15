/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2022 Patrick Dussud
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef DJ_JTAG_H
#define DJ_JTAG_H

#include <hardware/pio.h>

typedef struct dj_jtag_inst {
    PIO pio;
    uint sm;
    uint pin_tdi;
    uint pin_tdo;
    uint pin_tck;
    uint pin_tms;
    uint pin_rst;
    uint pin_trst;
} dj_jtag_inst_t;

void dj_jtag_init(dj_jtag_inst_t *jtag);

void jtag_set_clk_freq(const dj_jtag_inst_t *jtag, uint freq_khz);

void jtag_transfer(const dj_jtag_inst_t *jtag, uint32_t length,
                   const uint8_t *in, uint8_t *out);

uint8_t jtag_strobe(const dj_jtag_inst_t *jtag, uint32_t length, bool tms,
                    bool tdi);

void jtag_set_pin(const dj_jtag_inst_t *jtag, uint pin, bool value);
bool jtag_get_pin(const dj_jtag_inst_t *jtag, uint pin);

#endif // DJ_JTAG_H

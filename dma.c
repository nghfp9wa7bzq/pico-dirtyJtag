/*
 * Copyright (c) 2021 Patrick Dussud
 * Copyright (c) 2025 nghfp9wa7bzq@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "dma.h"


// Claim an unused DMA channel and configure it based on the parameters.
// Returns the channel number and also sets the config using a pointer.
uint claim_dma_ch(dma_channel_config *cc, uint8_t cc_tds, bool cc_ri, bool cc_wi, uint8_t cc_dreq,
                  const volatile void *cc_ra, volatile void *cc_wa, uint cc_tc, bool cc_t)
{
    uint ch = dma_claim_unused_channel(true);
    *cc = dma_channel_get_default_config(ch);
    channel_config_set_transfer_data_size(cc, cc_tds);
    channel_config_set_read_increment(cc, cc_ri);
    channel_config_set_write_increment(cc, cc_wi);
    channel_config_set_dreq(cc, cc_dreq);
    // false for trigger at the end means don't start the transfer yet.
    dma_channel_set_read_addr(ch, cc_ra, false);
    dma_channel_set_write_addr(ch, cc_wa, false);
    dma_channel_set_trans_count(ch, cc_tc, false);
    dma_channel_set_config(ch, cc, cc_t);
    
    return ch;
}

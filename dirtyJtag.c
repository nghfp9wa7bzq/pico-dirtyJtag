/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2025 Patrick Dussud
 * Copyright (c) 2021 jeanthom 
 * Copyright (c) 2023 David Williams (davidthings)
 * Copyright (c) 2023 Chandler Klüser
 * Copyright (c) 2024 DangerousPrototypes
 * Copyright (c) 2024 DESKTOP-M9CCUTI\ian
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

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pico/multicore.h"
#include "pio_jtag.h"
#include "cdc_uart.h"
#include "led.h"
#include "bsp/board.h"
#include "tusb.h"
#include "cmd.h"
#include "get_serial.h"
#include "dirtyJtagConfig.h"


pio_jtag_inst_t jtag = {
    .pio = pio0,
    .sm = 0
};

typedef uint8_t cmd_buffer[VENDOR_BUFFER_SIZE];
static cmd_buffer rx_buf;
static cmd_buffer tx_buf;
static uint32_t bytes_available = 0;

// Single CPU core case
#if !MULTICORE
void jtag_main_task()
{
    //If tud_task() is called and tud_vendor_read isn't called immediately (i.e before calling tud_task again)
    //after there is data available, there is a risk that data from 2 BULK OUT transaction will be (partially) combined into one
    //The DJTAG protocol does not tolerate this.
    tud_task();// tinyusb device task

    // Get the number of available bytes and only transfer that many,
    // instead of the whole buffer.
    bytes_available = 0;
    if (bytes_available = tud_vendor_available())
    {
        led_rx(1);
        uint count = tud_vendor_read(rx_buf, bytes_available);
        if (count != 0)
        {
            cmd_handle(&jtag, rx_buf, count, tx_buf);
        }
        led_rx(0);
    } else {
        // Note that we are prioritizing the JTAG interface.
        cdc_uart_task();
    }
}

// MULTICORE case
// This uses an array of buffers and the multicore fifos to delegate
// command handling to the second CPU core.
#else
static uint wr_buffer_number = 0;
static uint rd_buffer_number = 0;
typedef struct buffer_info
{
    volatile uint8_t count;
    volatile uint8_t busy;
    cmd_buffer buffer;
} buffer_info;

#define n_buffers (4)

buffer_info buffer_infos[n_buffers];

// After receiving commands, go to the next buffer.
void switch_buffer(uint *bpt)
{
    *bpt += 1;
    if (*bpt == n_buffers)
    {
        *bpt = 0;
    }
}

void jtag_main_task()
{
    if (multicore_fifo_rvalid())
    {
        //some command processing has been done
        uint rx_num = multicore_fifo_pop_blocking();
        buffer_info* bi = &buffer_infos[rx_num];
        bi->busy = false;
    }

    uint bnum = wr_buffer_number;
    if ((buffer_infos[bnum].busy == false))
    {
        //If tud_task() is called and tud_vendor_read isn't called immediately (i.e before calling tud_task again)
        //after there is data available, there is a risk that data from 2 BULK OUT transaction will be (partially) combined into one
        //The DJTAG protocol does not tolerate this.
        tud_task();// tinyusb device task

        // Get the number of available bytes and only transfer that many,
        // instead of the whole buffer.
        bytes_available = 0;
        if (bytes_available = tud_vendor_available())
        {
            led_rx(1);
            uint count = tud_vendor_read(buffer_infos[bnum].buffer, bytes_available);
            if (count != 0)
            {
                buffer_infos[bnum].count = count;
                buffer_infos[bnum].busy = true;
                switch_buffer(&wr_buffer_number);

                multicore_fifo_push_blocking(bnum);
            }
            led_rx(0);
        } else {
            // Note that we are prioritizing the JTAG interface.
            cdc_uart_task();
        }
    }
}

void core1_entry() {
    while (1)
    {
        uint rx_num = multicore_fifo_pop_blocking();
        buffer_info* bi = &buffer_infos[rx_num];
        assert (bi->busy);
        cmd_handle(&jtag, bi->buffer, bi->count, tx_buf);
        multicore_fifo_push_blocking(rx_num);
    }
}
#endif

//this is to work around the fact that tinyUSB does not handle setup request automatically
//Hence this boiler plate code
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
    if (stage != CONTROL_STAGE_SETUP) return true;
    return false;
}

int main()
{
    board_init();
    usb_serial_init();
    tusb_init();
    jtag_init(&jtag);
    led_init(LED_INVERTED, PIN_LED_TX, PIN_LED_RX, PIN_LED_ERROR);
    cdc_uart_init();

#if MULTICORE
    multicore_launch_core1(core1_entry);
#endif

    while (1) {
        jtag_main_task();
    }
}

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2025 Patrick Dussud
 * Copyright (c) 2023 David Williams (davidthings)
 * Copyright (c) 2025 nghfp9wa7bzq@gmail.com
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

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <bsp/board.h>
#include <tusb.h>

#include "dirtyJtag.h"
#include "dj_get_serial.h"
#include "dj_jtag.h"
#include "dj_led.h"
#include "dj_uart.h"

void main_loop(void)
{
    while (1) {
        dj_jtag_task();
        dj_uart_task();
    }
}

int main(void)
{
    board_init();
    usb_serial_init();
    tusb_init();

    dj_led_init(LED_INVERTED, PIN_LED_TX, PIN_LED_RX, PIN_LED_ERROR);
    dj_jtag_init();
    dj_uart_init();

#if MULTICORE
    // Run the same loop on core 0 and core 1.
    multicore_launch_core1(main_loop);
#endif

    main_loop();

    return 0;
}

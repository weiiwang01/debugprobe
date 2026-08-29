/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef PIO_UART_H
#define PIO_UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
  PIO_UART_PARITY_NONE,
  PIO_UART_PARITY_EVEN,
  PIO_UART_PARITY_ODD,
} pio_uart_parity_t;

void pio_uart_init(uint32_t baudrate);
void pio_update_pins(void);
void pio_uart_set_baudrate(uint32_t baudrate);
void pio_uart_set_format(unsigned int data_bits, unsigned int stop_bits,
                         pio_uart_parity_t parity);
void pio_uart_set_break(bool enabled);
bool pio_uart_is_readable(void);
uint8_t pio_uart_getc(void);
void pio_uart_write_blocking(const uint8_t *data, size_t length);

#endif

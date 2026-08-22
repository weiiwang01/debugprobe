/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Scott Shawcroft for Adafruit Industries
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
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
 *
 * SPDX-License-Identifier: MIT AND BSD-3-Clause
 */

#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <hardware/pio_instructions.h>

#include "pio_uart.h"
#include "pio_uart.pio.h"
#include "probe_config.h"

#define PIO_UART_CYCLES_PER_BIT 8u

#ifdef PROBE_UART_HWFC
#define PIO_UART_TX_PROGRAM pio_uart_tx_hwfc_program
#define PIO_UART_TX_WRAP pio_uart_tx_hwfc_wrap
#define PIO_UART_TX_WRAP_TARGET pio_uart_tx_hwfc_wrap_target
#define PIO_UART_TX_TWO_STOP pio_uart_tx_hwfc_offset_two_stop
#define PIO_UART_RX_PROGRAM pio_uart_rx_hwfc_program
#define PIO_UART_RX_WRAP pio_uart_rx_hwfc_wrap
#define PIO_UART_RX_WRAP_TARGET pio_uart_rx_hwfc_wrap_target
#else
#define PIO_UART_TX_PROGRAM pio_uart_tx_program
#define PIO_UART_TX_WRAP pio_uart_tx_wrap
#define PIO_UART_TX_WRAP_TARGET pio_uart_tx_wrap_target
#define PIO_UART_TX_TWO_STOP pio_uart_tx_offset_two_stop
#define PIO_UART_RX_PROGRAM pio_uart_rx_program
#define PIO_UART_RX_WRAP pio_uart_rx_wrap
#define PIO_UART_RX_WRAP_TARGET pio_uart_rx_wrap_target
#endif

static PIO uart_pio = PROBE_UART_PIO;
static uint tx_sm;
static uint rx_sm;
static int tx_offset = -1;
static int rx_offset = -1;
static uint32_t uart_baudrate;
static uint uart_data_bits = 8;
static uint uart_stop_bits = 1;
static pio_uart_parity_t uart_parity = PIO_UART_PARITY_NONE;
static bool break_enabled;

static uint
pio_uart_frame_bits(void)
{
  return uart_data_bits + (uart_parity != PIO_UART_PARITY_NONE);
}

static void
pio_uart_load_programs(void)
{
  tx_offset = pio_add_program(uart_pio, &PIO_UART_TX_PROGRAM);
  if (tx_offset < 0)
    panic("Unable to load PIO UART TX program");
  rx_offset = pio_add_program(uart_pio, &PIO_UART_RX_PROGRAM);
  if (rx_offset < 0)
    {
      pio_remove_program(uart_pio, &PIO_UART_TX_PROGRAM, (uint)tx_offset);
      tx_offset = -1;
      panic("Unable to load PIO UART RX program");
    }
}

static float
pio_uart_clkdiv(uint32_t baudrate)
{
  if (baudrate == 0)
    baudrate = 1;

  float divider = (float)clock_get_hz(clk_sys)
                  / ((float)PIO_UART_CYCLES_PER_BIT * (float)baudrate);
  if (divider < 1.0f)
    divider = 1.0f;
  if (divider > 65535.0f)
    divider = 65535.0f;
  return divider;
}

static uint
pio_uart_tx_entry(void)
{
  return (uint)tx_offset
         + (uart_stop_bits == 2 ? PIO_UART_TX_TWO_STOP
                                : PIO_UART_TX_WRAP_TARGET);
}

static void
pio_uart_set_frame_count(void)
{
  const uint instruction = pio_encode_set(pio_y, pio_uart_frame_bits() - 1);
  pio_sm_exec(uart_pio, tx_sm, instruction);
  pio_sm_exec(uart_pio, rx_sm, instruction);
}

static void
pio_uart_configure_state_machines(void)
{
  const uint sm_mask = (1u << tx_sm) | (1u << rx_sm);
  const float divider = pio_uart_clkdiv(uart_baudrate);
  const uint tx_entry = pio_uart_tx_entry();

  pio_set_sm_mask_enabled(uart_pio, sm_mask, false);
  pio_sm_clear_fifos(uart_pio, tx_sm);
  pio_sm_clear_fifos(uart_pio, rx_sm);

  pio_sm_config tx_config = pio_get_default_sm_config();
  sm_config_set_wrap(&tx_config, tx_entry, (uint)tx_offset + PIO_UART_TX_WRAP);
  sm_config_set_sideset(&tx_config, 2, true, false);
  sm_config_set_out_pins(&tx_config, PROBE_UART_TX, 1);
  sm_config_set_sideset_pins(&tx_config, PROBE_UART_TX);
  sm_config_set_out_shift(&tx_config, true, false, 32);
  sm_config_set_fifo_join(&tx_config, PIO_FIFO_JOIN_TX);
  sm_config_set_clkdiv(&tx_config, divider);
#ifdef PROBE_UART_HWFC
  sm_config_set_in_pins(&tx_config, PROBE_UART_CTS);
#endif
  pio_sm_init(uart_pio, tx_sm, tx_entry, &tx_config);

  pio_sm_config rx_config = pio_get_default_sm_config();
  sm_config_set_wrap(&rx_config, (uint)rx_offset + PIO_UART_RX_WRAP_TARGET,
                     (uint)rx_offset + PIO_UART_RX_WRAP);
  sm_config_set_in_pins(&rx_config, PROBE_UART_RX);
  sm_config_set_jmp_pin(&rx_config, PROBE_UART_RX);
  sm_config_set_in_shift(&rx_config, true, false, 32);
  sm_config_set_fifo_join(&rx_config, PIO_FIFO_JOIN_RX);
  sm_config_set_clkdiv(&rx_config, divider);
#ifdef PROBE_UART_HWFC
  sm_config_set_out_pins(&rx_config, PROBE_UART_RTS, 1);
  sm_config_set_mov_status(&rx_config, STATUS_RX_LESSTHAN, 7);
#endif
  pio_sm_init(uart_pio, rx_sm, (uint)rx_offset + PIO_UART_RX_WRAP_TARGET,
              &rx_config);
  pio_uart_set_frame_count();

  pio_sm_set_pins_with_mask(uart_pio, tx_sm, 1u << PROBE_UART_TX,
                            1u << PROBE_UART_TX);
#ifdef PROBE_UART_HWFC
  pio_sm_set_pins_with_mask(uart_pio, rx_sm, 0, 1u << PROBE_UART_RTS);
#endif
  pio_set_sm_mask_enabled(uart_pio, sm_mask, true);
  break_enabled = false;
}

void
pio_uart_init(uint32_t baudrate)
{
  tx_sm = pio_claim_unused_sm(uart_pio, true);
  rx_sm = pio_claim_unused_sm(uart_pio, true);

  pio_gpio_init(uart_pio, PROBE_UART_TX);
  pio_gpio_init(uart_pio, PROBE_UART_RX);
  pio_sm_set_consecutive_pindirs(uart_pio, tx_sm, PROBE_UART_TX, 1, true);
  pio_sm_set_consecutive_pindirs(uart_pio, rx_sm, PROBE_UART_RX, 1, false);
  gpio_pull_up(PROBE_UART_TX);
  gpio_pull_up(PROBE_UART_RX);

#ifdef PROBE_UART_HWFC
  pio_gpio_init(uart_pio, PROBE_UART_CTS);
  pio_sm_set_consecutive_pindirs(uart_pio, tx_sm, PROBE_UART_CTS, 1, false);
  gpio_pull_up(PROBE_UART_CTS);
  pio_gpio_init(uart_pio, PROBE_UART_RTS);
  pio_sm_set_consecutive_pindirs(uart_pio, rx_sm, PROBE_UART_RTS, 1, true);
#endif

  uart_baudrate = baudrate;
  pio_uart_load_programs();
  pio_uart_configure_state_machines();
}

void
pio_uart_set_baudrate(uint32_t baudrate)
{
  uart_baudrate = baudrate;
  pio_uart_configure_state_machines();
}

void
pio_uart_set_format(unsigned int data_bits, unsigned int stop_bits,
                    pio_uart_parity_t parity)
{
  uart_data_bits = data_bits;
  uart_stop_bits = stop_bits;
  uart_parity = parity;
  pio_uart_configure_state_machines();
}

void
pio_uart_set_break(bool enabled)
{
  if (enabled == break_enabled)
    return;

  pio_sm_set_enabled(uart_pio, tx_sm, false);
  pio_sm_clear_fifos(uart_pio, tx_sm);
  pio_sm_restart(uart_pio, tx_sm);
  pio_sm_set_pins_with_mask(uart_pio, tx_sm,
                            (enabled ? 0u : 1u) << PROBE_UART_TX,
                            1u << PROBE_UART_TX);
  if (!enabled)
    {
      pio_sm_exec(uart_pio, tx_sm, pio_encode_jmp(pio_uart_tx_entry()));
      pio_sm_set_enabled(uart_pio, tx_sm, true);
    }
  break_enabled = enabled;
}

bool
pio_uart_is_readable(void)
{
  return !pio_sm_is_rx_fifo_empty(uart_pio, rx_sm);
}

uint8_t
pio_uart_getc(void)
{
  const uint rx_frame_bits = pio_uart_frame_bits();
  const uint32_t mask = (1u << uart_data_bits) - 1u;
  return (uint8_t)((pio_sm_get(uart_pio, rx_sm) >> (32u - rx_frame_bits))
                   & mask);
}

static uint32_t
pio_uart_make_frame(uint8_t value)
{
  const uint32_t data_mask = (1u << uart_data_bits) - 1u;
  const uint32_t data = value & data_mask;
  uint32_t frame = data;

  if (uart_parity != PIO_UART_PARITY_NONE)
    {
      uint parity = __builtin_popcount(data) & 1u;
      if (uart_parity == PIO_UART_PARITY_ODD)
        parity ^= 1u;
      frame |= parity << uart_data_bits;
    }

  return frame;
}

void
pio_uart_write_blocking(const uint8_t *data, size_t length)
{
  if (break_enabled)
    return;
  for (size_t i = 0; i < length; ++i)
    pio_sm_put_blocking(uart_pio, tx_sm, pio_uart_make_frame(data[i]));
}

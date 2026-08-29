/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>

#include <hardware/gpio.h>

#include "DAP.h"
#include "cdc_uart.h"
#include "probe_config.h"
#include "probe.h"

static bool
pin_is_valid(uint8_t pin)
{
  return pin < NUM_BANK0_GPIOS && pin < 32;
}

static bool
pin_is_led(uint8_t pin)
{
#ifdef PROBE_USB_CONNECTED_LED
  if (pin == PROBE_USB_CONNECTED_LED)
    return true;
#endif
#ifdef PROBE_DAP_CONNECTED_LED
  if (pin == PROBE_DAP_CONNECTED_LED)
    return true;
#endif
#ifdef PROBE_DAP_RUNNING_LED
  if (pin == PROBE_DAP_RUNNING_LED)
    return true;
#endif
#ifdef PROBE_UART_RX_LED
  if (pin == PROBE_UART_RX_LED)
    return true;
#endif
#ifdef PROBE_UART_TX_LED
  if (pin == PROBE_UART_TX_LED)
    return true;
#endif
  return false;
}

static bool
bindings_are_valid(const uint8_t bindings[PROBE_FUNC_COUNT])
{
  for (uint i = 0; i < PROBE_FUNC_COUNT; ++i)
    {
      if (!pin_is_valid(bindings[i]) || pin_is_led(bindings[i]))
        return false;
      for (uint j = i + 1; j < PROBE_FUNC_COUNT; ++j)
        if (bindings[i] == bindings[j])
          return false;
    }

#ifdef PROBE_IO_OEN
  if (bindings[PROBE_FUNC_SWDIOEN] + 1 != bindings[PROBE_FUNC_SWCLK])
    return false;
#endif

  return true;
}

static bool
pin_is_used(uint8_t pin)
{
  for (uint i = 0; i < PROBE_FUNC_COUNT; ++i)
    if (probe_pin_mappings[i] == pin)
      return true;
  return false;
}

static void
release_old_pins(const uint8_t old_bindings[PROBE_FUNC_COUNT])
{
  for (uint i = 0; i < PROBE_FUNC_COUNT; ++i)
    if (!pin_is_used(old_bindings[i]))
      {
        gpio_deinit(old_bindings[i]);
#ifdef PROBE_PIN_SWDIOEN
        if (i != PROBE_FUNC_SWDIOEN)
#endif
          gpio_disable_pulls(old_bindings[i]);
      }
}

static bool
update_pin_bindings(const uint8_t request[PROBE_FUNC_COUNT])
{
  if (!bindings_are_valid(request))
    return false;

  uint8_t old_bindings[PROBE_FUNC_COUNT];
  for (uint i = 0; i < PROBE_FUNC_COUNT; ++i)
    {
      old_bindings[i] = probe_pin_mappings[i];
      probe_pin_mappings[i] = request[i];
    }

  probe_update_pins();
#ifdef PROBE_CDC_UART
  cdc_uart_update_pins();
#endif
  release_old_pins(old_bindings);
  return true;
}

/*
 * Vendor0 request:  [0x80, swclk, swdio, swdi?, swdioen?, reset?, uart_rx?, uart_tx?, uart_cts?, uart_rts?, uart_dtr?]
 * Vendor0 response: [0x80, DAP_OK|DAP_ERROR]
 */
uint32_t
DAP_ProcessVendorCommand(const uint8_t *request, uint8_t *response)
{
  const uint8_t command = request[0];
  response[0] = command;

  if (command != ID_DAP_Vendor0)
    {
      response[0] = ID_DAP_Invalid;
      return (1U << 16) | 1U;
    }

  response[1] = update_pin_bindings(request + 1) ? DAP_OK : DAP_ERROR;
  return (((uint32_t)(1 + PROBE_FUNC_COUNT)) << 16) | 2U;
}

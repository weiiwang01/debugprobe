/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
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

#ifndef PROBE_CONFIG_H_
#define PROBE_CONFIG_H_

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#if false
#define probe_info(format,args...) \
do { \
	vTaskSuspendAll(); \
	printf(format, ## args); \
	xTaskResumeAll(); \
} while (0)
#else
#define probe_info(format,...) ((void)0)
#endif


#if false
#define probe_debug(format,args...) \
do { \
	vTaskSuspendAll(); \
	printf(format, ## args); \
	xTaskResumeAll(); \
} while (0)
#else
#define probe_debug(format,...) ((void)0)
#endif

#if false
#define probe_dump(format,args...)\
do { \
	vTaskSuspendAll(); \
	printf(format, ## args); \
	xTaskResumeAll(); \
} while (0)
#else
#define probe_dump(format,...) ((void)0)
#endif

// TODO tie this up with PICO_BOARD defines in the main SDK

#ifdef DEBUG_ON_PICO 
#include "board_pico_config.h"
#else
#include "board_debug_probe_config.h"
#endif
//#include "board_example_config.h"

enum probe_function
{
#ifdef PROBE_PIN_SWCLK
  PROBE_FUNC_SWCLK,
#endif
#ifdef PROBE_PIN_SWDIO
  PROBE_FUNC_SWDIO,
#endif
#ifdef PROBE_PIN_SWDI
  PROBE_FUNC_SWDI,
#endif
#ifdef PROBE_PIN_SWDIOEN
  PROBE_FUNC_SWDIOEN,
#endif
#ifdef PROBE_PIN_RESET
  PROBE_FUNC_RESET,
#endif
#ifdef PROBE_UART_RX
  PROBE_FUNC_UART_RX,
#endif
#ifdef PROBE_UART_TX
  PROBE_FUNC_UART_TX,
#endif
#if defined(PROBE_UART_HWFC) && defined(PROBE_UART_CTS)
  PROBE_FUNC_UART_CTS,
#endif
#ifdef PROBE_UART_RTS
  PROBE_FUNC_UART_RTS,
#endif
#ifdef PROBE_UART_DTR
  PROBE_FUNC_UART_DTR,
#endif
  PROBE_FUNC_COUNT,
};

extern uint8_t probe_pin_mappings[PROBE_FUNC_COUNT];

// Add the configuration to binary information
void bi_decl_config();

#define PROTO_DAP_V1 1
#define PROTO_DAP_V2 2

// Interface config
#ifndef PROBE_DEBUG_PROTOCOL
#define PROBE_DEBUG_PROTOCOL PROTO_DAP_V2
#endif

#endif

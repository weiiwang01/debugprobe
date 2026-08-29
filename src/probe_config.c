#include "probe_config.h"
#include "pico/binary_info.h"
#include "probe/version.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

uint8_t probe_pin_mappings[PROBE_FUNC_COUNT] = {
#ifdef PROBE_PIN_SWCLK
    [PROBE_FUNC_SWCLK] = PROBE_PIN_SWCLK,
#endif
#ifdef PROBE_PIN_SWDIO
    [PROBE_FUNC_SWDIO] = PROBE_PIN_SWDIO,
#endif
#ifdef PROBE_PIN_SWDI
    [PROBE_FUNC_SWDI] = PROBE_PIN_SWDI,
#endif
#ifdef PROBE_PIN_SWDIOEN
    [PROBE_FUNC_SWDIOEN] = PROBE_PIN_SWDIOEN,
#endif
#ifdef PROBE_PIN_RESET
    [PROBE_FUNC_RESET] = PROBE_PIN_RESET,
#endif
#ifdef PROBE_UART_RX
    [PROBE_FUNC_UART_RX] = PROBE_UART_RX,
#endif
#ifdef PROBE_UART_TX
    [PROBE_FUNC_UART_TX] = PROBE_UART_TX,
#endif
#if defined(PROBE_UART_HWFC) && defined(PROBE_UART_CTS)
    [PROBE_FUNC_UART_CTS] = PROBE_UART_CTS,
#endif
#ifdef PROBE_UART_RTS
    [PROBE_FUNC_UART_RTS] = PROBE_UART_RTS,
#endif
#ifdef PROBE_UART_DTR
    [PROBE_FUNC_UART_DTR] = PROBE_UART_DTR,
#endif
};


void bi_decl_config()
{
    bi_decl(bi_program_version_string(PROBE_VERSION));
#ifdef PROBE_PIN_RESET
    bi_decl(bi_1pin_with_name(PROBE_PIN_RESET, "PROBE RESET"));
#endif

#ifdef PROBE_PIN_SWCLK
    bi_decl(bi_1pin_with_name(PROBE_PIN_SWCLK, "PROBE SWCLK"));
#endif

#ifdef PROBE_PIN_SWDIO
    bi_decl(bi_1pin_with_name(PROBE_PIN_SWDIO, "PROBE SWDIO"));
#endif

#ifdef PROBE_PIN_SWDI
    bi_decl(bi_1pin_with_name(PROBE_PIN_SWDI, "PROBE SWDI"));
#endif

#ifdef PROBE_PIN_SWDIOEN
    bi_decl(bi_1pin_with_name(PROBE_PIN_SWDIOEN, "PROBE SWDIOEN"));
#endif

#ifdef PROBE_CDC_UART
    bi_decl(bi_program_feature("PROBE UART PIO " STR(PROBE_UART_PIO)));
    bi_decl(bi_program_feature("PROBE UART BAUDRATE " STR(PROBE_UART_BAUDRATE)));
    bi_decl(bi_1pin_with_name(PROBE_UART_TX, "PROBE UART TX"));
    bi_decl(bi_1pin_with_name(PROBE_UART_RX, "PROBE UART RX"));
#endif

#ifdef PROBE_UART_CTS
    bi_decl(bi_1pin_with_name(PROBE_UART_CTS, "PROBE UART CTS"));
#endif
#ifdef PROBE_UART_RTS
    bi_decl(bi_1pin_with_name(PROBE_UART_RTS, "PROBE UART RTS"));
#endif
#ifdef PROBE_UART_DTR
    bi_decl(bi_1pin_with_name(PROBE_UART_DTR, "PROBE UART DTR"));
#endif

}

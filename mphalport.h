#include <stdio.h>
#include "ch32fun.h"

#ifdef CH570_CH572
#define LED_PIN   PA9
#else
#define LED_PIN   PA8
#endif

#define RX_BUF_SIZE 64
extern volatile uint8_t rx_buf[RX_BUF_SIZE];
extern volatile int rx_head;
extern volatile int rx_tail;


static inline mp_uint_t mp_hal_ticks_cpu(void) {
	return funSysTick32();
}

static inline mp_uint_t mp_hal_ticks_us(void) {
	return funSysTick32() * DELAY_US_TIME;
}

static inline mp_uint_t mp_hal_ticks_ms(void) {
	return funSysTick32() * DELAY_MS_TIME;
}

static inline void mp_hal_delay_us(mp_uint_t us) {
	Delay_Us(us);
}

static inline void mp_hal_delay_ms(mp_uint_t ms) {
	Delay_Ms(ms);
}

// stdout
static inline mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len) {
	for (size_t i = 0; i < len; i++) {
		putchar(str[i]); 
	}
	return len;
}

extern void poll_usbfs_input();
static inline int mp_hal_stdin_rx_chr(void) {
	char c = 0;
	while(1) {
#if defined(FUNCONF_USE_DEBUGPRINTF) && FUNCONF_USE_DEBUGPRINTF
		poll_input();
#endif
		poll_usbfs_input();
		if (rx_head != rx_tail) {
			c = rx_buf[rx_tail];
			rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
			return (c == '\n') ? '\r' : c;
		}
	}
}

// We can leave this empty for now, or use it to handle CTRL-C
static inline void mp_hal_set_interrupt_char(int c) {
	// If you want to support KeyboardInterrupt, you would store 'c'
}

#include <stdio.h>
#include "ch32fun.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

extern QueueHandle_t tty_rx_queue;

static inline mp_uint_t mp_hal_ticks_ms(void) {
	return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

static inline void mp_hal_delay_ms(mp_uint_t ms) {
	uint32_t ticks = ms / portTICK_PERIOD_MS;
	if (ticks == 0) ticks = 1;
	vTaskDelay(ticks);
}

// stdout
static inline mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len) {
	for (size_t i = 0; i < len; i++) {
		putchar(str[i]); 
	}
	return len;
}

static inline int mp_hal_stdin_rx_chr(void) {
    char c = 0;
    xQueueReceive(tty_rx_queue, &c, portMAX_DELAY);
    return c;
}

// We can leave this empty for now, or use it to handle CTRL-C
static inline void mp_hal_set_interrupt_char(int c) {
	// If you want to support KeyboardInterrupt, you would store 'c'
}

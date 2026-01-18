#include <stdio.h>
#include "ch32fun.h"

#include "py/mphal.h"
#include "py/builtin.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/stackctrl.h"
#include "shared/runtime/pyexec.h"

#if defined(FUNCONF_USE_USBPRINTF) && FUNCONF_USE_USBPRINTF
#include "fsusb.h"
#endif

extern int errno; // for libm
int *__errno(void) { return &errno; }

#if MICROPY_ENABLE_COMPILER
void do_str(const char *src, mp_parse_input_kind_t input_kind) {
	nlr_buf_t nlr;
	if (nlr_push(&nlr) == 0) {
		mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
		qstr source_name = lex->source_name;
		mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
		mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
		mp_call_function_0(module_fun);
		nlr_pop();
	} else {
		// uncaught exception
		mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
	}
}
#endif

#if MICROPY_ENABLE_GC
void gc_helper_collect_regs_and_stack(void);

void gc_collect(void) {
	gc_collect_start();
	gc_helper_collect_regs_and_stack();
	gc_collect_end();
}
#endif

volatile uint8_t rx_buf[RX_BUF_SIZE];
volatile int rx_head;
volatile int rx_tail;
void handle_input(int numbytes, uint8_t *data) {
	for (int i = 0; i < numbytes; i++) {
		int next = (rx_head + 1) % RX_BUF_SIZE;

		if (next != rx_tail) {
			rx_buf[rx_head] = data[i];
			rx_head = next;
		}
	}
}
#if defined(FUNCONF_USE_DEBUGPRINTF) && FUNCONF_USE_DEBUGPRINTF
void handle_debug_input(int numbytes, uint8_t *data) { handle_input(numbytes, data); }
#elif defined(FUNCONF_USE_USBPRINTF) && FUNCONF_USE_USBPRINTF
void handle_usbfs_input(int numbytes, uint8_t *data) { handle_input(numbytes, data); }
#endif

// __HIGH_CODE
static uint8_t mp_heap[MICROPY_HEAP_SIZE];
void micropython_task() {
	// 1. Initialize the stack limit so GC doesn't overflow
	volatile uint32_t sp;
	mp_stack_set_top((void*)&sp);
	mp_stack_set_limit((MICROPY_STACK_SIZE) - 512);

	// 2. Initialize the heap
#if MICROPY_ENABLE_GC
	gc_init(mp_heap, mp_heap +sizeof(mp_heap));
#endif

	// 3. Start
	mp_init();
	mp_hal_stdout_tx_strn("Booting MicroPython\r\n", 21);

	// This function will block inside mp_hal_stdin_rx_chr() waiting for input.
	// When it blocks, FreeRTOS will swap to other tasks.
	while (1) {
		if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
			if (pyexec_raw_repl() != 0) {
				break; // sys.exit() called
			}
		}
		else {
			if (pyexec_friendly_repl() != 0) {
				break; // sys.exit() called
			}
		}
	}
	
	mp_deinit();	
}


int main(void) {
	SystemInit();
	funGpioInitAll(); // no-op on ch5xx
	funPinMode(LED_PIN, GPIO_CFGLR_OUT_10Mhz_PP);

#if defined(FUNCONF_USE_USBPRINTF) && FUNCONF_USE_USBPRINTF
	USBFSSetup();
#endif

	printf("Booting MCU\n");

	while(1) {
		micropython_task();
	}
}


mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
	mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path) {
	return MP_IMPORT_STAT_NO_EXIST;
}

void nlr_jump_fail(void *val) {
	while (1);
}

void MP_NORETURN __fatal_error(const char *msg) {
	while (1);
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
	printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
	__fatal_error("Assertion failed");
}
#endif

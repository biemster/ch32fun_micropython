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
#include "shared/runtime/interrupt_char.h"

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

		if(data[i] == mp_interrupt_char) {
			mp_sched_keyboard_interrupt();
		}
	}
}
void handle_debug_input(int numbytes, uint8_t *data) { handle_input(numbytes, data); }
void handle_usbfs_input(int numbytes, uint8_t *data) { handle_input(numbytes, data); }

extern void poll_usbfs_input();
uint32_t last_usbfs_poll = 0;
void mp_hal_background_processing(void) {
	// THIS RUNS IN MICROPY_VM_HOOK_LOOP AFTER EVERY OPCODE, so we need to be really quick

	// Only poll every 1ms
	if (mp_hal_ticks_ms() != last_usbfs_poll) {
		last_usbfs_poll = mp_hal_ticks_ms();

#if defined(FUNCONF_USE_DEBUGPRINTF) && FUNCONF_USE_DEBUGPRINTF
		poll_input();
#endif
		poll_usbfs_input();
	}
}

static uint8_t mp_heap[MICROPY_HEAP_SIZE];
extern void execute_main_py(void);

// __HIGH_CODE // adds 1.2kB to RAM
void micropython_task() {

	// execute_main_py(); // this will prevent the REPL from showing if main.py blocks, so not beginner friendly

	// This loop will block inside mp_hal_stdin_rx_chr() waiting for input.
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

extern void usb_init();
int main(void) {
	SystemInit();
	funGpioInitAll(); // no-op on ch5xx

	usb_init();

	printf("Booting MCU\n");

	volatile uint32_t sp;
	mp_stack_set_top((void*)&sp);
	mp_stack_set_limit((MICROPY_STACK_SIZE) - 512);

#if MICROPY_ENABLE_GC
	gc_init(mp_heap, mp_heap +sizeof(mp_heap));
#endif

	mp_init();
	mp_hal_stdout_tx_strn("Booting MicroPython\r\n", 21);

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

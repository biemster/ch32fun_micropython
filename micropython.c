#include <stdio.h>
#include "ch32fun.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mpthreadport.h"

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
static char heap[MICROPY_HEAP_SIZE];
void gc_helper_get_regs_and_sp(mp_uint_t *regs);

void gc_helper_collect_regs_and_stack(void) {
    // Buffer to hold registers. 
    // RISC-V 32I saves: ra, sp, gp, tp, s0-s11 (approx 16 registers).
    // We allocate 32 just to be safe and maintain alignment.
    mp_uint_t regs[32];

    // Call the assembly function. 
    // This saves CPU registers into the 'regs' array.
    // Crucially, it saves the current Stack Pointer (SP) into regs[1].
    gc_helper_get_regs_and_sp(regs);

    // 1. Scan the captured registers (Live pointers in CPU)
    gc_collect_root((void**)regs, sizeof(regs) / sizeof(mp_uint_t));

    // 2. Scan the Stack
    // MicroPython requires us to know the top of the stack.
    // In a Bare Metal port, this is usually &_estack.
    // In FreeRTOS, this must be the top of the CURRENT TASK's stack.
    void **stack_top = (void**)MP_STATE_THREAD(stack_top);
    
    // The bottom of the stack is the SP we just captured in regs[1]
    void **stack_ptr = (void**)regs[1];

    // Sanity check: Ensure stack grows down (Standard RISC-V) 
    // and that SP is within bounds.
    if (stack_ptr < stack_top) {
        gc_collect_root(stack_ptr, stack_top - stack_ptr);
    }
}

void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
    gc_collect_end();
}
#endif

TaskHandle_t MicroPythonTask_Handler;
QueueHandle_t tty_rx_queue; // for the REPL

__HIGH_CODE
void micropython_task(void *pvParameters) {
	// 1. Initialize the stack limit so GC doesn't overflow
	char *sp = (char*)__builtin_frame_address(0);
	mp_stack_set_top(sp);
	mp_stack_set_limit((MICROPY_STACK_SIZE *4) - 1024);

	// 2. Initialize the heap
#if MICROPY_ENABLE_GC
	gc_init(heap, heap + sizeof(heap));
#endif

	// 3. Start
	mp_init();
	mp_hal_stdout_tx_strn("Booting MicroPython...\r\n", 24);

	// This function will block inside mp_hal_stdin_rx_chr() waiting for input.
	// When it blocks, FreeRTOS will swap to other tasks.
	for (;;) {
		if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
			if (pyexec_raw_repl() != 0) {
				break; // sys.exit() called
			}
		} else {
			if (pyexec_friendly_repl() != 0) {
				break; // sys.exit() called
			}
		}
	}
	
	mp_deinit();	
	vTaskDelete(NULL); 
}


int main(void) {
	SystemInit();
	funGpioInitAll(); // no-op on ch5xx

#if defined(FUNCONF_USE_USBPRINTF) && FUNCONF_USE_USBPRINTF
	USBFSSetup();
#endif

	xTaskCreate((TaskFunction_t)micropython_task,
				(const char *)"micropython",
				(uint16_t)MICROPY_STACK_SIZE,
				(void *)NULL,
				(UBaseType_t)MICROPY_TASK_PRIO,
				(TaskHandle_t *)&MicroPythonTask_Handler);

	vTaskStartScheduler();

	while(1); // shouldn't run at here!!
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

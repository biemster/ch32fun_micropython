#include <stdint.h>

// options to control how MicroPython is built

// Use the minimal starting configuration (disables all optional features).
#define MICROPY_CONFIG_ROM_LEVEL            (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)

// You can disable the built-in MicroPython compiler by setting the following
// config option to 0.  If you do this then you won't get a REPL prompt, but you
// will still be able to execute pre-compiled scripts, compiled with mpy-cross.
#define MICROPY_ENABLE_COMPILER             (1)
#define MICROPY_PY_BUILTINS_EVAL_EXEC       (1)

#define MICROPY_PY_THREAD                   (0)
#define MICROPY_PY_THREAD_GIL               (0) // Global Interpreter Lock
#define MICROPY_TASK_PRIO                   (5) // FreeRTOS task priority
#define MICROPY_ENABLE_GC                   (1)
#define MICROPY_ENABLE_SCHEDULER            (1)
#define MICROPY_GCREGS_SETJMP               (1)
#define MICROPY_HELPER_REPL                 (1)
#define MICROPY_REPL_EVENT_DRIVEN           (0)
#define MICROPY_MODULE_FROZEN_MPY           (0)
#define MICROPY_MODULE_FROZEN_STR           (0)
// #define MICROPY_QSTR_EXTRA_POOL             mp_qstr_frozen_const_pool
#define MICROPY_ENABLE_EXTERNAL_IMPORT      (1)
#define MICROPY_HEAP_SIZE                   (3 * 1024) // malloced on the FreeRTOS heap
#define MICROPY_STACK_SIZE                  (2 * 1024)
#define MICROPY_BYTES_PER_GC_BLOCK          (16) // or 8, matches FreeRTOS alignment

// Use the minimum headroom in the chunk allocator for parse nodes.
#define MICROPY_ALLOC_PARSE_CHUNK_INIT      (16)

// --- Memory Saving Configuration ---
// 1. Disable Floating Point (Saves huge code space and RAM)
#define MICROPY_FLOAT_IMPL                  (MICROPY_FLOAT_IMPL_FLOAT)

// 2. Disable Long Integers (Only use 31-bit small ints)
#define MICROPY_LONGINT_IMPL                (MICROPY_LONGINT_IMPL_LONGLONG)

// 3. Disable Error Reporting details
#define MICROPY_ERROR_REPORTING             (MICROPY_ERROR_REPORTING_TERSE)
#define MICROPY_ENABLE_SOURCE_LINE          (0)
#define MICROPY_ENABLE_DOC_STRING           (0)

// 4. Disable expensive core features
#define MICROPY_PY_BUILTINS_COMPLEX         (1) // doesn't work because of floats for some reason
#define MICROPY_PY_BUILTINS_SET             (1)
#define MICROPY_PY_BUILTINS_SLICE           (1)
#define MICROPY_PY_BUILTINS_PROPERTY        (1)
#define MICROPY_PY_BUILTINS_MIN_MAX         (1)
#define MICROPY_PY_BUILTINS_STR_COUNT       (1)
#define MICROPY_PY_BUILTINS_STR_OP_MODULO   (1) // No % string formatting
#define MICROPY_PY_SYS                      (1) // Needed for startup
#define MICROPY_PY_TIME                     (1)
#define MICROPY_PY_TIME_GMTIME              (0)
#define MICROPY_PY_TIME_LOCALTIME           (0)
#define MICROPY_PY_TIME_TIME_NS             (0)
#define MICROPY_PY_MATH                     (1)
#define MICROPY_PY_CMATH                    (1)
#define MICROPY_PY_IO                       (0) // Use print() but no file objects
#define MICROPY_PY_STRUCT                   (1)
#define MICROPY_CPYTHON_COMPAT              (1)

// 5. Shrink Internal Structures
#define MICROPY_ALLOC_PATH_MAX              (32)
#define MICROPY_QSTR_BYTES_IN_HASH          (1)

#define MICROPY_PY_SYS_MODULES              (1)
#define MICROPY_PY_SYS_EXIT                 (1)
#define MICROPY_PY_SYS_PATH                 (1)
#define MICROPY_PY_SYS_ARGV                 (1)

// type definitions for the specific machine

typedef long mp_off_t;

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MICROPY_HW_BOARD_NAME               "ch32fun"
#define MICROPY_HW_MCU_NAME                 "wch-riscv"

#define MP_STATE_PORT                       MP_STATE_VM

// for open('main.py').read()
extern const struct _mp_obj_fun_builtin_var_t mp_builtin_open_obj;

#define MICROPY_PORT_BUILTINS \
	{ MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&mp_builtin_open_obj) },

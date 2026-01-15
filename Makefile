all : flash

TARGET:=micropython
TARGET_MCU:=CH585
TARGET_MCU_PACKAGE:=CH585M

CH32FUN_PATH = $(abspath ../ch32fun/)
FREERTOS_PATH = $(abspath ../ch32fun_freertos/FreeRTOS/)
MICROPYTHON_PATH = $(abspath ../micropython/)

EXTRA_CFLAGS += \
	-I. \
	-I$(FREERTOS_PATH) \
	-I$(FREERTOS_PATH)/include \
	-I$(FREERTOS_PATH)/portable/GCC/RISC-V \
	-I$(MICROPYTHON_PATH) \
	-I$(MICROPYTHON_PATH)/py

FREERTOS_SRC += \
	$(FREERTOS_PATH)/croutine.c \
	$(FREERTOS_PATH)/event_groups.c \
	$(FREERTOS_PATH)/list.c \
	$(FREERTOS_PATH)/queue.c \
	$(FREERTOS_PATH)/stream_buffer.c \
	$(FREERTOS_PATH)/tasks.c \
	$(FREERTOS_PATH)/timers.c \
	$(FREERTOS_PATH)/portable/GCC/RISC-V/port.c \
	$(FREERTOS_PATH)/portable/MemMang/heap_4.c \
	$(FREERTOS_PATH)/portable/GCC/RISC-V/portASM.S

MICROPYTHON_SRC_RUNTIME += \
	$(MICROPYTHON_PATH)/py/runtime.c \
	$(MICROPYTHON_PATH)/py/mpstate.c \
	$(MICROPYTHON_PATH)/py/repl.c \
	$(MICROPYTHON_PATH)/py/builtinimport.c \
	$(MICROPYTHON_PATH)/py/builtinevex.c \
	$(MICROPYTHON_PATH)/py/nlr.c \
	$(MICROPYTHON_PATH)/py/nlrrv32.c \
	$(MICROPYTHON_PATH)/py/malloc.c \
	$(MICROPYTHON_PATH)/py/gc.c \
	$(MICROPYTHON_PATH)/py/pystack.c \
	$(MICROPYTHON_PATH)/py/qstr.c \
	$(MICROPYTHON_PATH)/py/vstr.c \
	$(MICROPYTHON_PATH)/py/mpprint.c \
	$(MICROPYTHON_PATH)/py/unicode.c \
	$(MICROPYTHON_PATH)/py/mpz.c \
	$(MICROPYTHON_PATH)/py/reader.c \
	$(MICROPYTHON_PATH)/py/lexer.c \
	$(MICROPYTHON_PATH)/py/parse.c \
	$(MICROPYTHON_PATH)/py/scope.c \
	$(MICROPYTHON_PATH)/py/compile.c \
	$(MICROPYTHON_PATH)/py/emitcommon.c \
	$(MICROPYTHON_PATH)/py/emitbc.c \
	$(MICROPYTHON_PATH)/py/asmbase.c \
	$(MICROPYTHON_PATH)/py/parsenumbase.c \
	$(MICROPYTHON_PATH)/py/emitglue.c \
	$(MICROPYTHON_PATH)/py/parsenum.c \
	$(MICROPYTHON_PATH)/py/bc.c \
	$(MICROPYTHON_PATH)/py/binary.c \
	$(MICROPYTHON_PATH)/py/smallint.c \
	$(MICROPYTHON_PATH)/py/map.c \
	$(MICROPYTHON_PATH)/py/vm.c \
	$(MICROPYTHON_PATH)/py/nativeglue.c \
	$(MICROPYTHON_PATH)/py/pairheap.c \
	$(MICROPYTHON_PATH)/py/ringbuf.c \
	$(MICROPYTHON_PATH)/py/stackctrl.c \
	$(MICROPYTHON_PATH)/py/argcheck.c \
	$(MICROPYTHON_PATH)/py/warning.c \
	$(MICROPYTHON_PATH)/py/profile.c \
	$(MICROPYTHON_PATH)/py/scheduler.c \
	$(MICROPYTHON_PATH)/py/opmethods.c \
	$(MICROPYTHON_PATH)/py/sequence.c \
	$(MICROPYTHON_PATH)/py/stream.c

MICROPYTHON_SRC_OBJECTS += \
	$(MICROPYTHON_PATH)/py/obj.c \
	$(MICROPYTHON_PATH)/py/objarray.c \
	$(MICROPYTHON_PATH)/py/objattrtuple.c \
	$(MICROPYTHON_PATH)/py/objbool.c \
	$(MICROPYTHON_PATH)/py/objboundmeth.c \
	$(MICROPYTHON_PATH)/py/objcell.c \
	$(MICROPYTHON_PATH)/py/objclosure.c \
	$(MICROPYTHON_PATH)/py/objdict.c \
	$(MICROPYTHON_PATH)/py/objenumerate.c \
	$(MICROPYTHON_PATH)/py/objexcept.c \
	$(MICROPYTHON_PATH)/py/objfilter.c \
	$(MICROPYTHON_PATH)/py/objfun.c \
	$(MICROPYTHON_PATH)/py/objgenerator.c \
	$(MICROPYTHON_PATH)/py/objgetitemiter.c \
	$(MICROPYTHON_PATH)/py/objint.c \
	$(MICROPYTHON_PATH)/py/objint_mpz.c \
	$(MICROPYTHON_PATH)/py/objlist.c \
	$(MICROPYTHON_PATH)/py/objmap.c \
	$(MICROPYTHON_PATH)/py/objmodule.c \
	$(MICROPYTHON_PATH)/py/objobject.c \
	$(MICROPYTHON_PATH)/py/objpolyiter.c \
	$(MICROPYTHON_PATH)/py/objproperty.c \
	$(MICROPYTHON_PATH)/py/objnone.c \
	$(MICROPYTHON_PATH)/py/objnamedtuple.c \
	$(MICROPYTHON_PATH)/py/objrange.c \
	$(MICROPYTHON_PATH)/py/objreversed.c \
	$(MICROPYTHON_PATH)/py/objset.c \
	$(MICROPYTHON_PATH)/py/objsingleton.c \
	$(MICROPYTHON_PATH)/py/objslice.c \
	$(MICROPYTHON_PATH)/py/objstr.c \
	$(MICROPYTHON_PATH)/py/objstrunicode.c \
	$(MICROPYTHON_PATH)/py/objstringio.c \
	$(MICROPYTHON_PATH)/py/objtuple.c \
	$(MICROPYTHON_PATH)/py/objtype.c \
	$(MICROPYTHON_PATH)/py/objzip.c

MICROPYTHON_SRC_STDLIB += \
	$(MICROPYTHON_PATH)/py/modbuiltins.c \
	$(MICROPYTHON_PATH)/py/modsys.c \
	$(MICROPYTHON_PATH)/py/modgc.c \
	$(MICROPYTHON_PATH)/py/modio.c \
	$(MICROPYTHON_PATH)/py/moderrno.c \
	$(MICROPYTHON_PATH)/py/modstruct.c \
	$(MICROPYTHON_PATH)/py/modmath.c \
	$(MICROPYTHON_PATH)/py/modthread.c \
	$(MICROPYTHON_PATH)/py/modmicropython.c


MICROPYTHON_SRC_HELPERS += \
	$(MICROPYTHON_PATH)/shared/libc/string0.c \
	$(MICROPYTHON_PATH)/shared/runtime/pyexec.c \
	$(MICROPYTHON_PATH)/shared/runtime/interrupt_char.c \
	$(MICROPYTHON_PATH)/shared/runtime/stdout_helpers.c \
	$(MICROPYTHON_PATH)/shared/runtime/sys_stdio_mphal.c \
	$(MICROPYTHON_PATH)/shared/readline/readline.c \
	$(MICROPYTHON_PATH)/shared/runtime/gchelper_rv32i.s \
	./mpthreadport.c

MICROPYTHON_SRC += \
	$(MICROPYTHON_SRC_RUNTIME) \
	$(MICROPYTHON_SRC_OBJECTS) \
	$(MICROPYTHON_SRC_STDLIB) \
	$(MICROPYTHON_SRC_HELPERS)

ADDITIONAL_C_FILES += $(FREERTOS_SRC) $(MICROPYTHON_SRC)

GENHDR_DIR = genhdr
include ./mp.mk
include $(CH32FUN_PATH)/ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean
	rm -rf $(GENHDR_DIR)

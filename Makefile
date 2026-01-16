all : flash

TARGET:=micropython
TARGET_MCU:=CH570
TARGET_MCU_PACKAGE:=CH570D

CH32FUN_PATH = $(abspath ../ch32fun/)
FREERTOS_PATH = $(abspath ../ch32fun_freertos/FreeRTOS/)
MICROPYTHON_PATH = $(abspath ../micropython/)

EXTRA_CFLAGS += \
	-I. \
	-I$(FREERTOS_PATH) \
	-I$(FREERTOS_PATH)/include \
	-I$(FREERTOS_PATH)/portable/GCC/RISC-V \
	-I$(MICROPYTHON_PATH) \
	-I$(MICROPYTHON_PATH)/py \
	-DNDEBUG

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

MICROPYTHON_SRC += \
	$(MICROPYTHON_PATH)/py/*.c \
	$(MICROPYTHON_PATH)/shared/runtime/gchelper_rv32i.s \
	$(MICROPYTHON_PATH)/shared/runtime/pyexec.c \
	$(MICROPYTHON_PATH)/shared/runtime/interrupt_char.c \
	$(MICROPYTHON_PATH)/shared/runtime/stdout_helpers.c \
	$(MICROPYTHON_PATH)/shared/runtime/sys_stdio_mphal.c \
	$(MICROPYTHON_PATH)/shared/readline/readline.c

ADDITIONAL_C_FILES += $(FREERTOS_SRC) $(MICROPYTHON_SRC)
LDFLAGS += -lm

GENHDR_DIR = genhdr
include ./mp.mk
include $(CH32FUN_PATH)/ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean
	rm -rf $(GENHDR_DIR)

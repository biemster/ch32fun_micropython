all : flash

TARGET:=micropython
TARGET_MCU:=CH570
TARGET_MCU_PACKAGE:=CH570D
#TARGET_MCU:=CH32V208
#TARGET_MCU_PACKAGE:=CH32V208WBU6

CH32FUN_PATH = $(abspath ../ch32fun/)
MICROPYTHON_PATH = $(abspath ../micropython/)

EXTRA_CFLAGS += \
	-I. \
	-I$(MICROPYTHON_PATH) \
	-I$(MICROPYTHON_PATH)/py \
	-DNDEBUG

# upstream sources
MICROPYTHON_SRC += \
	$(MICROPYTHON_PATH)/py/*.c \
	$(MICROPYTHON_PATH)/shared/runtime/gchelper_generic.c \
	$(MICROPYTHON_PATH)/shared/runtime/pyexec.c \
	$(MICROPYTHON_PATH)/shared/runtime/interrupt_char.c \
	$(MICROPYTHON_PATH)/shared/runtime/stdout_helpers.c \
	$(MICROPYTHON_PATH)/shared/runtime/sys_stdio_mphal.c \
	$(MICROPYTHON_PATH)/shared/readline/readline.c \
	$(MICROPYTHON_PATH)/extmod/modtime.c

# modules for the port
MICROPYTHON_SRC += \
	./usbfs_cdc_msc.c \
	./machine_pin.c

ADDITIONAL_C_FILES += $(MICROPYTHON_SRC) $(MICROPYTHON_PORT_MODULES)
LDFLAGS += -lm

GENHDR_DIR = genhdr
include ./mp.mk
include $(CH32FUN_PATH)/ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean
	rm -rf $(GENHDR_DIR)

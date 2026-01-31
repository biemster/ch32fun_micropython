#ifndef MICROPY_INCLUDED_WCH_MODCH32FUN_H
#define MICROPY_INCLUDED_WCH_MODCH32FUN_H

#include "py/runtime.h"
#include "ch32fun.h"

// ==========================================================================
// Memory Map Definitions
// ==========================================================================
// These are needed by RAM accessor (modch32fun.c) and Flash (ch32fun_flash.c)

// these should be filled from ch32fun linker script
#define RAM_START      0x20000000
#define RAM_SIZE       (12 * 1024)
#define RAM_END        (RAM_START + RAM_SIZE)

#define FLASH_START    0x00000000
#define FLASH_SIZE     RAM_START // not the real size, but we might want to poke beyond what the DS says
#define FLASH_END      (FLASH_START + FLASH_SIZE)

// ==========================================================================
// Shared Helper Functions
// ==========================================================================
// Defined in modch32fun.c, used by ch32fun_flash.c
void ch32fun_check_addr(uintptr_t addr, size_t len, uintptr_t start, uintptr_t end);

// ==========================================================================
// External Object/Type Declarations
// ==========================================================================

// Defined in ch32fun_ch5xx.c
extern const mp_obj_type_t ch32fun_ch5xx_type;
extern const mp_obj_base_t ch5xx_obj; // The singleton instance

// Defined in ch32fun_flash.c
extern const mp_obj_type_t ch32fun_flash_type;

// Defined in ch32fun_isler.c
extern const mp_obj_type_t ch32fun_isler_type;

// Defined in ch32fun_nfc.c
extern const mp_obj_type_t ch32fun_nfc_type;

#endif // MICROPY_INCLUDED_WCH_MODCH32FUN_H

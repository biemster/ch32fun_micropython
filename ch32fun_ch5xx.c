#include "py/runtime.h"
#include "py/objstr.h"
#include "modch32fun.h"
#include <string.h>

// ==========================================================================
// ch5xx Register Accessor ch32fun.ch5xx.R32_*
// ==========================================================================

enum { W8, W16, W32 };

typedef struct {
	qstr name;
	uintptr_t addr;
	uint8_t width;
} reg_entry_t;

// --- THE REGISTER TABLE ---
static const reg_entry_t ch5xx_reg_table[] = {
	// System Registers
	// { MP_QSTR_R32_TMR_CONTROL , (uintptr_t)&R32_TMR_CONTROL , W32 },
	#include "ch32fun_regdefs.h"
};

static void ch5xx_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
	// 1. Search for the register name in our table
	const reg_entry_t *reg = NULL;
	for (size_t i = 0; i < (sizeof(ch5xx_reg_table) / sizeof(reg_entry_t)); i++) {
		if (ch5xx_reg_table[i].name == attr) {
			reg = &ch5xx_reg_table[i];
			break;
		}
	}

	if (reg == NULL) {
		return; // Lookup failed
	}

	// 2. Handle Load (Read)
	if (dest[0] == MP_OBJ_NULL) {
		mp_int_t val = 0;
		if (reg->width == W32) val = *(volatile uint32_t *)reg->addr;
		else if (reg->width == W16) val = *(volatile uint16_t *)reg->addr;
		else val = *(volatile uint8_t *)reg->addr;
		dest[0] = mp_obj_new_int_from_uint(val);
	}
	// 3. Handle Store (Write)
	else if (dest[1] != MP_OBJ_NULL) {
		mp_int_t val = mp_obj_get_int(dest[1]);
		if (reg->width == W32) *(volatile uint32_t *)reg->addr = (uint32_t)val;
		else if (reg->width == W16) *(volatile uint16_t *)reg->addr = (uint16_t)val;
		else *(volatile uint8_t *)reg->addr = (uint8_t)val;
		dest[0] = MP_OBJ_NULL; // Indicate success
	}
}

// NOTE: Not static, exposed in header
MP_DEFINE_CONST_OBJ_TYPE(
	ch32fun_ch5xx_type,
	MP_QSTR_ch5xx,
	MP_TYPE_FLAG_NONE,
	attr, ch5xx_attr
);

// Create the Singleton instance (Not static)
const mp_obj_base_t ch32fun_ch5xx_obj = { &ch32fun_ch5xx_type };

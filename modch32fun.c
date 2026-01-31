#include "py/runtime.h"
#include "modch32fun.h"
#include <string.h>

// ==========================================================================
// Shared Helpers
// ==========================================================================

void ch32fun_check_addr(uintptr_t addr, size_t len, uintptr_t start, uintptr_t end) {
	if (addr < start || (addr + len) > end || (addr + len) < addr) {
		mp_raise_ValueError(MP_ERROR_TEXT("address out of bounds"));
	}
}

// ==========================================================================
// RAM Accessor Object (ch32fun.RAM[])
// ==========================================================================

typedef struct _ch32fun_ram_obj_t {
	mp_obj_base_t base;
} ch32fun_ram_obj_t;

const mp_obj_type_t ch32fun_ram_type;

static mp_obj_t ram_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
	// 1. Handle Slicing: RAM[start:end]
	if (mp_obj_is_type(index, &mp_type_slice)) {
		mp_obj_slice_t *slice = MP_OBJ_TO_PTR(index);

		if (slice->step != mp_const_none && mp_obj_get_int(slice->step) != 1) {
			 mp_raise_msg(&mp_type_NotImplementedError, MP_ERROR_TEXT("only step=1 supported"));
		}

		mp_int_t start_addr = mp_obj_get_int(slice->start);
		mp_int_t stop_addr = mp_obj_get_int(slice->stop);
		size_t len = stop_addr - start_addr;

		// Handle offset vs absolute
		if (start_addr < RAM_START) {
			start_addr += RAM_START;
			stop_addr += RAM_START;
		}

		ch32fun_check_addr(start_addr, len, RAM_START, RAM_END);

		if (value == MP_OBJ_SENTINEL) {
			// Read Slice
			return mp_obj_new_memoryview('B', len, (void*)(uintptr_t)start_addr);
		}
		else if (value == MP_OBJ_NULL) {
			mp_raise_TypeError(MP_ERROR_TEXT("cannot delete memory"));
		}
		else {
			// Write Slice
			mp_buffer_info_t bufinfo;
			mp_get_buffer_raise(value, &bufinfo, MP_BUFFER_READ);
			if (bufinfo.len != len) {
				mp_raise_ValueError(MP_ERROR_TEXT("slice assignment size mismatch"));
			}
			memcpy((void*)(uintptr_t)start_addr, bufinfo.buf, len);
		}
	}
	else {
		// 2. Handle Single Address: RAM[addr]
		size_t addr = mp_obj_get_int(index);
		addr += (addr < RAM_START) ? RAM_START : 0;
		ch32fun_check_addr(addr, 1, RAM_START, RAM_END);

		volatile uint8_t *ptr = (volatile uint8_t *)(uintptr_t)addr;

		if (value == MP_OBJ_SENTINEL) {
			return MP_OBJ_NEW_SMALL_INT(*ptr);
		}
		else {
			*ptr = (uint8_t)mp_obj_get_int(value);
		}
	}
	return mp_const_none;
}

static const ch32fun_ram_obj_t ch32fun_ram_obj = {{&ch32fun_ram_type}};

MP_DEFINE_CONST_OBJ_TYPE(
	ch32fun_ram_type,
	MP_QSTR_RAM,
	MP_TYPE_FLAG_NONE,
	subscr, ram_subscr
);

// ==========================================================================
// Main ch32fun Module Definition
// ==========================================================================

static const mp_rom_map_elem_t ch32fun_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_ch32fun) },

	// RAM is internal to this file
	{ MP_ROM_QSTR(MP_QSTR_RAM),         MP_ROM_PTR(&ch32fun_ram_obj) },

	// These are external from other files
	{ MP_ROM_QSTR(MP_QSTR_ch5xx),       MP_ROM_PTR(&ch5xx_obj) },
	{ MP_ROM_QSTR(MP_QSTR_ch5xx_flash), MP_ROM_PTR(&ch32fun_flash_type) },
	{ MP_ROM_QSTR(MP_QSTR_iSLER),       MP_ROM_PTR(&ch32fun_isler_type) },
	{ MP_ROM_QSTR(MP_QSTR_NFC),         MP_ROM_PTR(&ch32fun_nfc_type) },
};
static MP_DEFINE_CONST_DICT(ch32fun_module_globals, ch32fun_module_globals_table);

const mp_obj_module_t mp_module_ch32fun = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&ch32fun_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_ch32fun, mp_module_ch32fun);

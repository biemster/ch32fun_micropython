#include "py/runtime.h"
#include "modch32fun.h"

// ==========================================================================
// ch5xx_flash Submodule
// ==========================================================================

static mp_obj_t fun_flash_erase(mp_obj_t addr_in) {
	uint32_t addr = mp_obj_get_int(addr_in);
	// FLASH_Unlock_Fast();
	// FLASH_ErasePage_Fast(addr);
	// FLASH_Lock_Fast();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(fun_flash_erase_obj, fun_flash_erase);

// Usage: read(addr) or read(addr, len)
static mp_obj_t fun_flash_read(size_t n_args, const mp_obj_t *args) {
	uint32_t addr = mp_obj_get_int(args[0]);

	if (n_args == 1) {
		ch32fun_check_addr(addr, 1, FLASH_START, FLASH_END);
		uint8_t val = *(volatile uint8_t*)(uintptr_t)addr;
		return MP_OBJ_NEW_SMALL_INT(val);
	}
	else {
		size_t len = mp_obj_get_int(args[1]);
		ch32fun_check_addr(addr, len, FLASH_START, FLASH_END);
		// XIP Memoryview
		return mp_obj_new_memoryview('B', len, (void*)(uintptr_t)addr);
	}
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fun_flash_read_obj, 1, 2, fun_flash_read);

static mp_obj_t fun_flash_write(mp_obj_t addr_in, mp_obj_t data_in) {
	uint32_t addr = mp_obj_get_int(addr_in);

	if (mp_obj_is_int(data_in)) {
		ch32fun_check_addr(addr, 4, FLASH_START, FLASH_END);
		// uint32_t val = mp_obj_get_int(data_in);
		// FLASH_ProgramWord(addr, val);
	}
	else {
		mp_buffer_info_t bufinfo;
		mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);
		ch32fun_check_addr(addr, bufinfo.len, FLASH_START, FLASH_END);
		// Programming loop...
	}
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(fun_flash_write_obj, fun_flash_write);

static const mp_rom_map_elem_t flash_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_erase), MP_ROM_PTR(&fun_flash_erase_obj) },
	{ MP_ROM_QSTR(MP_QSTR_read),  MP_ROM_PTR(&fun_flash_read_obj) },
	{ MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&fun_flash_write_obj) },
};
static MP_DEFINE_CONST_DICT(flash_locals_dict, flash_locals_dict_table);

// NOTE: Not static
MP_DEFINE_CONST_OBJ_TYPE(
	ch32fun_flash_type,
	MP_QSTR_ch5xx_flash,
	MP_TYPE_FLAG_NONE,
	locals_dict, &flash_locals_dict
);

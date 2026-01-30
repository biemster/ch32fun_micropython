#include "py/runtime.h"
#include "py/objstr.h"
#include "ch32fun.h"
#include <string.h>
#include <stdio.h>

// these should be filled from ch32fun linker script
#define RAM_START      0x20000000
#define RAM_SIZE       (12 * 1024)
#define RAM_END        (RAM_START + RAM_SIZE)

#define FLASH_START    0x00000000
#define FLASH_SIZE     RAM_START // not the real size, but we might want to poke beyond what the DS says
#define FLASH_END      (FLASH_START + FLASH_SIZE)

// Helper to validate pointer
static void check_addr(uintptr_t addr, size_t len, uintptr_t start, uintptr_t end) {
	if (addr < start || (addr + len) > end || (addr + len) < addr) {
		mp_raise_ValueError(MP_ERROR_TEXT("address out of bounds"));
	}
}

// ==========================================================================
// 1. RAM Accessor Object (ch32fun.RAM[])
// ==========================================================================

// Define a minimal object type for the RAM accessor
typedef struct _ch32fun_ram_obj_t {
	mp_obj_base_t base;
} ch32fun_ram_obj_t;

const mp_obj_type_t ch32fun_ram_type;

// Handle RAM[addr] or RAM[start:end]
static mp_obj_t ram_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
	// 1. Handle Slicing: RAM[start:end] -> Returns memoryview
	if (mp_obj_is_type(index, &mp_type_slice)) {
		mp_obj_slice_t *slice = MP_OBJ_TO_PTR(index);
		mp_obj_t o_start = slice->start;
		mp_obj_t o_stop = slice->stop;
		mp_obj_t o_step = slice->step;

		if (o_step != mp_const_none && mp_obj_get_int(o_step) != 1) {
			 mp_raise_msg(&mp_type_NotImplementedError, MP_ERROR_TEXT("only step=1 supported"));
		}

		mp_int_t start_addr = mp_obj_get_int(o_start);
		mp_int_t stop_addr = mp_obj_get_int(o_stop);
		size_t len = stop_addr - start_addr;
		if (start_addr < RAM_START) {
			start_addr += RAM_START;
			stop_addr += RAM_START;
		}

		check_addr(start_addr, len, RAM_START, RAM_END);

		if (value == MP_OBJ_SENTINEL) {
			// Read Slice -> Return memoryview
			return mp_obj_new_memoryview('B', len, (void*)(uintptr_t)start_addr);
		}
		else if (value == MP_OBJ_NULL) {
			mp_raise_TypeError(MP_ERROR_TEXT("cannot delete memory"));
		}
		else {
			// Write Slice: RAM[x:y] = b'123'
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
		addr += (addr < RAM_START) ? RAM_START : 0; // allow reading address 0x0 as start of RAM
		check_addr(addr, 1, RAM_START, RAM_END);

		volatile uint8_t *ptr = (volatile uint8_t *)(uintptr_t)addr;

		if (value == MP_OBJ_SENTINEL) {
			// Load
			return MP_OBJ_NEW_SMALL_INT(*ptr);
		}
		else {
			// Store
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
// 2. iSLER Submodule (Static Methods)
// ==========================================================================

static mp_obj_t fun_isler_init(void) {
	// Call ch32fun iSLER initialization
	// iSLER_Init(); 
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(fun_isler_init_obj, fun_isler_init);

static mp_obj_t fun_isler_tx(mp_obj_t data_in) {
	mp_buffer_info_t bufinfo;
	mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);

	// iSLER_Tx(bufinfo.buf, bufinfo.len);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(fun_isler_tx_obj, fun_isler_tx);

static mp_obj_t fun_isler_rx(void) {
	// uint8_t *data;
	// size_t len;
	// len = iSLER_Rx(&data); // Pseudocode based on expected ch32fun API

	// Return empty bytes for now as placeholder
	return mp_obj_new_bytes(NULL, 0); 
}
static MP_DEFINE_CONST_FUN_OBJ_0(fun_isler_rx_obj, fun_isler_rx);

static const mp_rom_map_elem_t isler_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&fun_isler_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_tx),   MP_ROM_PTR(&fun_isler_tx_obj) },
	{ MP_ROM_QSTR(MP_QSTR_rx),   MP_ROM_PTR(&fun_isler_rx_obj) },
};
static MP_DEFINE_CONST_DICT(isler_locals_dict, isler_locals_dict_table);

static MP_DEFINE_CONST_OBJ_TYPE(
	ch32fun_isler_type,
	MP_QSTR_iSLER,
	MP_TYPE_FLAG_NONE,
	locals_dict, &isler_locals_dict
);

// ==========================================================================
// 3. NFC Submodule
// ==========================================================================

static mp_obj_t fun_nfc_init(void) {
	// NFC_Init();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(fun_nfc_init_obj, fun_nfc_init);

static const mp_rom_map_elem_t nfc_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&fun_nfc_init_obj) },
};
static MP_DEFINE_CONST_DICT(nfc_locals_dict, nfc_locals_dict_table);

static MP_DEFINE_CONST_OBJ_TYPE(
	ch32fun_nfc_type,
	MP_QSTR_NFC,
	MP_TYPE_FLAG_NONE,
	locals_dict, &nfc_locals_dict
);

// ==========================================================================
// 4. ch5xx_flash Submodule
// ==========================================================================

static mp_obj_t fun_flash_erase(mp_obj_t addr_in) {
	uint32_t addr = mp_obj_get_int(addr_in);

	// FLASH_Unlock_Fast();
	// FLASH_ErasePage_Fast(addr);
	// FLASH_Lock_Fast();

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(fun_flash_erase_obj, fun_flash_erase);

// Usage: 
// 1. read(addr) -> returns int (byte)
// 2. read(addr, len) -> returns memoryview (efficient)
static mp_obj_t fun_flash_read(size_t n_args, const mp_obj_t *args) {
	uint32_t addr = mp_obj_get_int(args[0]);

	if (n_args == 1) {
		// Single byte read
		check_addr(addr, 1, FLASH_START, FLASH_END);
		uint8_t val = *(volatile uint8_t*)(uintptr_t)addr;
		return MP_OBJ_NEW_SMALL_INT(val);
	}
	else {
		// Buffer read (Memoryview)
		size_t len = mp_obj_get_int(args[1]);
		check_addr(addr, len, FLASH_START, FLASH_END);

		// Return a memoryview pointing directly to Flash (XIP)
		// This creates no copy, extremely RAM efficient
		return mp_obj_new_memoryview('B', len, (void*)(uintptr_t)addr);
	}
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fun_flash_read_obj, 1, 2, fun_flash_read);

static mp_obj_t fun_flash_write(mp_obj_t addr_in, mp_obj_t data_in) {
	uint32_t addr = mp_obj_get_int(addr_in);

	if (mp_obj_is_int(data_in)) {
		// Write 32-bit word? Or byte? ch32fun usually does 32-bit programming
		// Let's assume user sends an integer to write as a word
		check_addr(addr, 4, FLASH_START, FLASH_END);
		// uint32_t val = mp_obj_get_int(data_in);
		// FLASH_ProgramWord(addr, val);
	}
	else {
		// Write buffer
		mp_buffer_info_t bufinfo;
		mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);
		check_addr(addr, bufinfo.len, FLASH_START, FLASH_END);

		// Loop and write
		// uint8_t *p = bufinfo.buf;
		// for(int i=0; i<bufinfo.len; i++) ...
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

static MP_DEFINE_CONST_OBJ_TYPE(
	ch32fun_flash_type,
	MP_QSTR_ch5xx_flash,
	MP_TYPE_FLAG_NONE,
	locals_dict, &flash_locals_dict
);

// ==========================================================================
// Main ch32fun Module
// ==========================================================================

static const mp_rom_map_elem_t ch32fun_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_ch32fun) },

	// RAM Accessor Instance
	{ MP_ROM_QSTR(MP_QSTR_RAM),         MP_ROM_PTR(&ch32fun_ram_obj) },

	// Sub-modules (exposed as types/classes)
	{ MP_ROM_QSTR(MP_QSTR_iSLER),       MP_ROM_PTR(&ch32fun_isler_type) },
	{ MP_ROM_QSTR(MP_QSTR_NFC),         MP_ROM_PTR(&ch32fun_nfc_type) },
	{ MP_ROM_QSTR(MP_QSTR_ch5xx_flash), MP_ROM_PTR(&ch32fun_flash_type) },
};
static MP_DEFINE_CONST_DICT(ch32fun_module_globals, ch32fun_module_globals_table);

const mp_obj_module_t mp_module_ch32fun = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&ch32fun_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_ch32fun, mp_module_ch32fun);

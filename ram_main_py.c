#include "py/runtime.h"
#include "py/objstr.h"
#include "py/compile.h"  // mp_compile()
#include "py/lexer.h"    // mp_lexer_t, mp_lexer_new_from_str_len()
#include "py/parse.h"    // mp_parse(), mp_parse_tree_t, MP_PARSE_FILE_INPUT
#include "py/mperrno.h" // Defines MP_ENOENT
#include <string.h>

extern uint8_t msc_ram_disk[];
extern uint32_t active_file_size;

void execute_main_py(void) {
	if (active_file_size == 0) return;

	nlr_buf_t nlr;
	if (nlr_push(&nlr) == 0) {
		// Create Lexer DIRECTLY from the RAM buffer (Zero copy)
		mp_lexer_t *lex = mp_lexer_new_from_str_len(
			MP_QSTR_main_dot_py, 
			(char*)msc_ram_disk, 
			active_file_size, 
			0
		);

		// Parse and Compile
		qstr source_name = lex->source_name;
		mp_parse_tree_t parse_tree = mp_parse(lex, MP_PARSE_FILE_INPUT);
		mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);

		// Execute
		mp_call_function_0(module_fun);

		nlr_pop();
	}
	else {
		// Print error if script crashes
		mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
	}
}

// --- 1. The RamFile Object ---
typedef struct _ram_file_obj_t {
	mp_obj_base_t base;
	uint32_t pos;
} ram_file_obj_t;

// --- 2. The .read() method ---
static mp_obj_t ram_file_read(size_t n_args, const mp_obj_t *args) {
	ram_file_obj_t *self = MP_OBJ_TO_PTR(args[0]);
	
	// Check if we are at end of file
	if (self->pos >= active_file_size) {
		return mp_const_empty_bytes; // Return ""
	}
	
	// Calculate size to read
	uint32_t len = active_file_size - self->pos;
	
	// Create a Python String (Copies data from buffer to Heap)
	// Warning: This requires enough Heap for the whole file!
	mp_obj_t ret = mp_obj_new_str((char*)msc_ram_disk + self->pos, len);
	
	self->pos += len;
	return ret;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ram_file_read_obj, 1, 2, ram_file_read);

// --- 3. The .close() method (Dummy) ---
static mp_obj_t ram_file_close(mp_obj_t self_in) {
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(ram_file_close_obj, ram_file_close);

// --- 4. Class Dictionary ---
static const mp_rom_map_elem_t ram_file_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_read),     MP_ROM_PTR(&ram_file_read_obj) },
	{ MP_ROM_QSTR(MP_QSTR_close),    MP_ROM_PTR(&ram_file_close_obj) },
	// Context Manager support: "with open(...) as f:"
	{ MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&mp_identity_obj) },
	{ MP_ROM_QSTR(MP_QSTR___exit__),  MP_ROM_PTR(&ram_file_close_obj) },
};
static MP_DEFINE_CONST_DICT(ram_file_locals_dict, ram_file_locals_dict_table);

// --- 5. Type Definition ---
MP_DEFINE_CONST_OBJ_TYPE(
	mp_type_ram_file,
	MP_QSTR_TextIOWrapper,
	MP_TYPE_FLAG_NONE,
	// Slots follow:
	locals_dict, &ram_file_locals_dict
);

// --- 6. The open() function implementation ---
mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
	const char *filename = mp_obj_str_get_str(args[0]);
	
	// Check if it is the magic file
	if (strcmp(filename, "main.py") == 0) {
		ram_file_obj_t *self = m_new_obj(ram_file_obj_t);
		self->base.type = &mp_type_ram_file;
		self->pos = 0;
		return MP_OBJ_FROM_PTR(self);
	}
	
	// File not found
	mp_raise_OSError(MP_ENOENT); 
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

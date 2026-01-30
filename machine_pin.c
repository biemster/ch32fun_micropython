#include "py/runtime.h"
#include "ch32fun.h"

// ==========================================================================
// Pin Object Definition
// ==========================================================================
typedef struct _machine_pin_obj_t {
	mp_obj_base_t base;
	uint32_t pin_id; // The hardware pin number used by ch32fun
} machine_pin_obj_t;

const mp_obj_type_t machine_pin_type;

// ==========================================================================
// Pin Class Methods
// ==========================================================================

// Constructor: p = machine.Pin(pin_id, mode)
static mp_obj_t machine_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	mp_arg_check_num(n_args, n_kw, 1, 2, false);

	// 1. Create the object
	machine_pin_obj_t *self = m_new_obj(machine_pin_obj_t);
	self->base.type = &machine_pin_type;
	self->pin_id = mp_obj_get_int(args[0]);

	// 2. Configure Mode if provided (IN=0, OUT=1)
	if (n_args >= 2) {
		uint8_t mode = mp_obj_get_int(args[1]);
		funPinMode(self->pin_id, mode);
	}

	return MP_OBJ_FROM_PTR(self);
}

// method: Pin.value([x])
static mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
	machine_pin_obj_t *self = MP_OBJ_TO_PTR(args[0]);
	
	if (n_args == 1) {
		// Get Value
		int val = funDigitalRead(self->pin_id);
		return MP_OBJ_NEW_SMALL_INT(val);
	} else {
		// Set Value
		int val = mp_obj_get_int(args[1]);
		funDigitalWrite(self->pin_id, val);
		return mp_const_none;
	}
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

// method: Pin.on()
static mp_obj_t machine_pin_on(mp_obj_t self_in) {
	machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
	funDigitalWrite(self->pin_id, FUN_HIGH);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_on_obj, machine_pin_on);

// method: Pin.off()
static mp_obj_t machine_pin_off(mp_obj_t self_in) {
	machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
	funDigitalWrite(self->pin_id, FUN_LOW);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_off_obj, machine_pin_off);

// --- Pin Class Lookup Table ---
static const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },
	{ MP_ROM_QSTR(MP_QSTR_on),    MP_ROM_PTR(&machine_pin_on_obj) },
	{ MP_ROM_QSTR(MP_QSTR_off),   MP_ROM_PTR(&machine_pin_off_obj) },
	// Constants for Mode
	{ MP_ROM_QSTR(MP_QSTR_IN),    MP_ROM_INT(GPIO_CFGLR_IN_FLOAT) },
	{ MP_ROM_QSTR(MP_QSTR_OUT),   MP_ROM_INT(GPIO_CFGLR_OUT_50Mhz_PP) },
	// Pins
	#include "ch32fun_pindefs.h" // pull in the generated P[ABCD]xx macros
};
static MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

// --- Pin Type Definition ---
MP_DEFINE_CONST_OBJ_TYPE(
	machine_pin_type,
	MP_QSTR_Pin,
	MP_TYPE_FLAG_NONE,
	make_new, machine_pin_make_new,
	locals_dict, &machine_pin_locals_dict
);

// ==========================================================================
// Machine Module Definition
// ==========================================================================

static const mp_rom_map_elem_t machine_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_machine) },
	{ MP_ROM_QSTR(MP_QSTR_Pin),      MP_ROM_PTR(&machine_pin_type) },
};
static MP_DEFINE_CONST_DICT(machine_module_globals, machine_module_globals_table);

const mp_obj_module_t mp_module_machine = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&machine_module_globals,
};

// Register the module so 'import machine' works
MP_REGISTER_MODULE(MP_QSTR_machine, mp_module_machine);

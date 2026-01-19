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
	{ MP_ROM_QSTR(MP_QSTR_PA0),   MP_ROM_INT(PA0) },
	{ MP_ROM_QSTR(MP_QSTR_PA1),   MP_ROM_INT(PA1) },
	{ MP_ROM_QSTR(MP_QSTR_PA2),   MP_ROM_INT(PA2) },
	{ MP_ROM_QSTR(MP_QSTR_PA3),   MP_ROM_INT(PA3) },
	{ MP_ROM_QSTR(MP_QSTR_PA4),   MP_ROM_INT(PA4) },
	{ MP_ROM_QSTR(MP_QSTR_PA5),   MP_ROM_INT(PA5) },
	{ MP_ROM_QSTR(MP_QSTR_PA6),   MP_ROM_INT(PA6) },
	{ MP_ROM_QSTR(MP_QSTR_PA7),   MP_ROM_INT(PA7) },
	{ MP_ROM_QSTR(MP_QSTR_PA8),   MP_ROM_INT(PA8) },
	{ MP_ROM_QSTR(MP_QSTR_PA9),   MP_ROM_INT(PA9) },
	{ MP_ROM_QSTR(MP_QSTR_PA10),   MP_ROM_INT(PA10) },
	{ MP_ROM_QSTR(MP_QSTR_PA11),   MP_ROM_INT(PA11) },
	{ MP_ROM_QSTR(MP_QSTR_PA12),   MP_ROM_INT(PA12) },
	{ MP_ROM_QSTR(MP_QSTR_PA13),   MP_ROM_INT(PA13) },
	{ MP_ROM_QSTR(MP_QSTR_PA14),   MP_ROM_INT(PA14) },
	{ MP_ROM_QSTR(MP_QSTR_PA15),   MP_ROM_INT(PA15) },
#ifdef PB0
	{ MP_ROM_QSTR(MP_QSTR_PB0),   MP_ROM_INT(PB0) },
	{ MP_ROM_QSTR(MP_QSTR_PB1),   MP_ROM_INT(PB1) },
	{ MP_ROM_QSTR(MP_QSTR_PB2),   MP_ROM_INT(PB2) },
	{ MP_ROM_QSTR(MP_QSTR_PB3),   MP_ROM_INT(PB3) },
	{ MP_ROM_QSTR(MP_QSTR_PB4),   MP_ROM_INT(PB4) },
	{ MP_ROM_QSTR(MP_QSTR_PB5),   MP_ROM_INT(PB5) },
	{ MP_ROM_QSTR(MP_QSTR_PB6),   MP_ROM_INT(PB6) },
	{ MP_ROM_QSTR(MP_QSTR_PB7),   MP_ROM_INT(PB7) },
	{ MP_ROM_QSTR(MP_QSTR_PB8),   MP_ROM_INT(PB8) },
	{ MP_ROM_QSTR(MP_QSTR_PB9),   MP_ROM_INT(PB9) },
	{ MP_ROM_QSTR(MP_QSTR_PB10),   MP_ROM_INT(PB10) },
	{ MP_ROM_QSTR(MP_QSTR_PB11),   MP_ROM_INT(PB11) },
	{ MP_ROM_QSTR(MP_QSTR_PB12),   MP_ROM_INT(PB12) },
	{ MP_ROM_QSTR(MP_QSTR_PB13),   MP_ROM_INT(PB13) },
	{ MP_ROM_QSTR(MP_QSTR_PB14),   MP_ROM_INT(PB14) },
	{ MP_ROM_QSTR(MP_QSTR_PB15),   MP_ROM_INT(PB15) },
#ifdef PB16
	{ MP_ROM_QSTR(MP_QSTR_PB16),   MP_ROM_INT(PB16) },
	{ MP_ROM_QSTR(MP_QSTR_PB17),   MP_ROM_INT(PB17) },
	{ MP_ROM_QSTR(MP_QSTR_PB18),   MP_ROM_INT(PB18) },
	{ MP_ROM_QSTR(MP_QSTR_PB19),   MP_ROM_INT(PB19) },
	{ MP_ROM_QSTR(MP_QSTR_PB20),   MP_ROM_INT(PB20) },
	{ MP_ROM_QSTR(MP_QSTR_PB21),   MP_ROM_INT(PB21) },
	{ MP_ROM_QSTR(MP_QSTR_PB22),   MP_ROM_INT(PB22) },
	{ MP_ROM_QSTR(MP_QSTR_PB23),   MP_ROM_INT(PB23) },
#endif
#endif
#ifdef PC0
	{ MP_ROM_QSTR(MP_QSTR_PC0),   MP_ROM_INT(PC0) },
	{ MP_ROM_QSTR(MP_QSTR_PC1),   MP_ROM_INT(PC1) },
	{ MP_ROM_QSTR(MP_QSTR_PC2),   MP_ROM_INT(PC2) },
	{ MP_ROM_QSTR(MP_QSTR_PC3),   MP_ROM_INT(PC3) },
	{ MP_ROM_QSTR(MP_QSTR_PC4),   MP_ROM_INT(PC4) },
	{ MP_ROM_QSTR(MP_QSTR_PC5),   MP_ROM_INT(PC5) },
	{ MP_ROM_QSTR(MP_QSTR_PC6),   MP_ROM_INT(PC6) },
	{ MP_ROM_QSTR(MP_QSTR_PC7),   MP_ROM_INT(PC7) },
	{ MP_ROM_QSTR(MP_QSTR_PC8),   MP_ROM_INT(PC8) },
	{ MP_ROM_QSTR(MP_QSTR_PC9),   MP_ROM_INT(PC9) },
	{ MP_ROM_QSTR(MP_QSTR_PC10),   MP_ROM_INT(PC10) },
	{ MP_ROM_QSTR(MP_QSTR_PC11),   MP_ROM_INT(PC11) },
	{ MP_ROM_QSTR(MP_QSTR_PC12),   MP_ROM_INT(PC12) },
	{ MP_ROM_QSTR(MP_QSTR_PC13),   MP_ROM_INT(PC13) },
	{ MP_ROM_QSTR(MP_QSTR_PC14),   MP_ROM_INT(PC14) },
	{ MP_ROM_QSTR(MP_QSTR_PC15),   MP_ROM_INT(PC15) },
#endif
#ifdef PD0
	{ MP_ROM_QSTR(MP_QSTR_PD0),   MP_ROM_INT(PD0) },
	{ MP_ROM_QSTR(MP_QSTR_PD1),   MP_ROM_INT(PD1) },
	{ MP_ROM_QSTR(MP_QSTR_PD2),   MP_ROM_INT(PD2) },
	{ MP_ROM_QSTR(MP_QSTR_PD3),   MP_ROM_INT(PD3) },
	{ MP_ROM_QSTR(MP_QSTR_PD4),   MP_ROM_INT(PD4) },
	{ MP_ROM_QSTR(MP_QSTR_PD5),   MP_ROM_INT(PD5) },
	{ MP_ROM_QSTR(MP_QSTR_PD6),   MP_ROM_INT(PD6) },
	{ MP_ROM_QSTR(MP_QSTR_PD7),   MP_ROM_INT(PD7) },
	{ MP_ROM_QSTR(MP_QSTR_PD8),   MP_ROM_INT(PD8) },
	{ MP_ROM_QSTR(MP_QSTR_PD9),   MP_ROM_INT(PD9) },
	{ MP_ROM_QSTR(MP_QSTR_PD10),   MP_ROM_INT(PD10) },
	{ MP_ROM_QSTR(MP_QSTR_PD11),   MP_ROM_INT(PD11) },
	{ MP_ROM_QSTR(MP_QSTR_PD12),   MP_ROM_INT(PD12) },
	{ MP_ROM_QSTR(MP_QSTR_PD13),   MP_ROM_INT(PD13) },
	{ MP_ROM_QSTR(MP_QSTR_PD14),   MP_ROM_INT(PD14) },
	{ MP_ROM_QSTR(MP_QSTR_PD15),   MP_ROM_INT(PD15) },
#endif
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

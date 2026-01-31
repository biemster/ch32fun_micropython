#include "py/runtime.h"
#include "modmachine.h"

// ==========================================================================
// Machine Module Definition
// ==========================================================================

static const mp_rom_map_elem_t machine_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_machine) },

	// Add the Pin class from machine_pin.c
	{ MP_ROM_QSTR(MP_QSTR_Pin),      MP_ROM_PTR(&machine_pin_type) },
	{ MP_ROM_QSTR(MP_QSTR_Signal),   MP_ROM_PTR(&machine_signal_type) },

	// Later
	// { MP_ROM_QSTR(MP_QSTR_ADC),   MP_ROM_PTR(&machine_adc_type) },
	// { MP_ROM_QSTR(MP_QSTR_SPI),   MP_ROM_PTR(&machine_spi_type) },
};
static MP_DEFINE_CONST_DICT(machine_module_globals, machine_module_globals_table);

const mp_obj_module_t mp_module_machine = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&machine_module_globals,
};

// Register the module so 'import machine' works
MP_REGISTER_MODULE(MP_QSTR_machine, mp_module_machine);

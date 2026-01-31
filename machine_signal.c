#include "py/runtime.h"
#include "modmachine.h"

typedef struct _machine_signal_obj_t {
	mp_obj_base_t base;
	mp_obj_t pin_obj; // The underlying Pin object
	bool invert;
} machine_signal_obj_t;

// Signal(pin_obj, invert=False)
static mp_obj_t machine_signal_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
	enum { ARG_pin_obj, ARG_invert };
	static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_pin_obj, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
		{ MP_QSTR_invert,  MP_ARG_BOOL, {.u_bool = false} },
	};

	mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

	machine_signal_obj_t *self = m_new_obj(machine_signal_obj_t);
	self->base.type = &machine_signal_type;
	self->pin_obj = args[ARG_pin_obj].u_obj;
	self->invert = args[ARG_invert].u_bool;

	return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t machine_signal_value(size_t n_args, const mp_obj_t *args) {
	machine_signal_obj_t *self = MP_OBJ_TO_PTR(args[0]);

	mp_obj_t dest[3];
	mp_load_method(self->pin_obj, MP_QSTR_value, dest);

	if (n_args == 1) {
		// Getter: dest[0]=method, dest[1]=pin_instance
		mp_obj_t res = mp_call_method_n_kw(0, 0, dest);
		int val = mp_obj_get_int(res);
		if (self->invert) {
			val = !val;
		}
		return MP_OBJ_NEW_SMALL_INT(val);
	}
	else {
		// Setter
		int val = mp_obj_get_int(args[1]);
		if (self->invert) {
			val = !val;
		}
		dest[2] = MP_OBJ_NEW_SMALL_INT(val);
		mp_call_method_n_kw(1, 0, dest);
		return mp_const_none;
	}
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_signal_value_obj, 1, 2, machine_signal_value);

static mp_obj_t machine_signal_on(mp_obj_t self_in) {
	machine_signal_obj_t *self = MP_OBJ_TO_PTR(self_in);
	// Determine what "on" means physically
	int phys_val = self->invert ? 0 : 1;

	// Call pin.value(phys_val)
	mp_obj_t dest[3];
	mp_load_method(self->pin_obj, MP_QSTR_value, dest);
	dest[2] = MP_OBJ_NEW_SMALL_INT(phys_val);
	mp_call_method_n_kw(1, 0, dest);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_signal_on_obj, machine_signal_on);

static mp_obj_t machine_signal_off(mp_obj_t self_in) {
	machine_signal_obj_t *self = MP_OBJ_TO_PTR(self_in);
	// Determine what "off" means physically
	int phys_val = self->invert ? 1 : 0;

	mp_obj_t dest[3];
	mp_load_method(self->pin_obj, MP_QSTR_value, dest);
	dest[2] = MP_OBJ_NEW_SMALL_INT(phys_val);
	mp_call_method_n_kw(1, 0, dest);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_signal_off_obj, machine_signal_off);

static const mp_rom_map_elem_t machine_signal_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_signal_value_obj) },
	{ MP_ROM_QSTR(MP_QSTR_on),    MP_ROM_PTR(&machine_signal_on_obj) },
	{ MP_ROM_QSTR(MP_QSTR_off),   MP_ROM_PTR(&machine_signal_off_obj) },
};
static MP_DEFINE_CONST_DICT(machine_signal_locals_dict, machine_signal_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
	machine_signal_type,
	MP_QSTR_Signal,
	MP_TYPE_FLAG_NONE,
	make_new, machine_signal_make_new,
	locals_dict, &machine_signal_locals_dict
);

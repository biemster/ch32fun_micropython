#include "py/runtime.h"
#include "modch32fun.h"

// ==========================================================================
// iSLER Submodule
// ==========================================================================

static mp_obj_t fun_isler_init(void) {
	// iSLER_Init();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(fun_isler_init_obj, fun_isler_init);

static mp_obj_t fun_isler_rx(void) {
	// Return empty bytes placeholder
	return mp_obj_new_bytes(NULL, 0);
}
static MP_DEFINE_CONST_FUN_OBJ_0(fun_isler_rx_obj, fun_isler_rx);

static mp_obj_t fun_isler_tx(mp_obj_t data_in) {
	mp_buffer_info_t bufinfo;
	mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);
	// iSLER_Tx(bufinfo.buf, bufinfo.len);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(fun_isler_tx_obj, fun_isler_tx);

static mp_obj_t fun_isler_adv(mp_obj_t data_in) {
	mp_buffer_info_t bufinfo;
	mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);
	// iSLER_Adv(bufinfo.buf, bufinfo.len);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(fun_isler_adv_obj, fun_isler_adv);

static const mp_rom_map_elem_t isler_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&fun_isler_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_rx),   MP_ROM_PTR(&fun_isler_rx_obj) },
	{ MP_ROM_QSTR(MP_QSTR_tx),   MP_ROM_PTR(&fun_isler_tx_obj) },
	{ MP_ROM_QSTR(MP_QSTR_adv),   MP_ROM_PTR(&fun_isler_adv_obj) },
};
static MP_DEFINE_CONST_DICT(isler_locals_dict, isler_locals_dict_table);

// NOTE: Not static
MP_DEFINE_CONST_OBJ_TYPE(
	ch32fun_isler_type,
	MP_QSTR_iSLER,
	MP_TYPE_FLAG_NONE,
	locals_dict, &isler_locals_dict
);

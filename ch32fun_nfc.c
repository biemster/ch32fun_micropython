#include "py/runtime.h"
#include "modch32fun.h"

// ==========================================================================
// NFC Submodule
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

// NOTE: Not static
MP_DEFINE_CONST_OBJ_TYPE(
    ch32fun_nfc_type,
    MP_QSTR_NFC,
    MP_TYPE_FLAG_NONE,
    locals_dict, &nfc_locals_dict
);

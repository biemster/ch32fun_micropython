#ifndef MICROPY_INCLUDED_WCH_MODMACHINE_H
#define MICROPY_INCLUDED_WCH_MODMACHINE_H

#include "py/obj.h"
#include "ch32fun.h"

typedef struct _machine_pin_obj_t {
	mp_obj_base_t base;
	uint32_t pin_id; 
} machine_pin_obj_t;

extern const mp_obj_type_t machine_pin_type;
extern const mp_obj_type_t machine_signal_type;

#endif // MICROPY_INCLUDED_WCH_MODMACHINE_H

#include "py/runtime.h"
#include "py/ringbuf.h"
#include "py/objstr.h"
#include "modch32fun.h"

// ==========================================================================
// iSLER Submodule
// ==========================================================================
void LLE_IRQHandler(void) __attribute__((used)); // keep the linker happy
#define ISLER_CALLBACK mod_isler_rx_isr
void ISLER_CALLBACK(void); // this should have been in extralibs/iSLER.h
#include "iSLER.h"

// ==========================================================================
// RX Ring Buffer Implementation
// ==========================================================================
// We need a buffer to store packets because Python might not read them fast enough.
// Size: 512 bytes (approx 2 large packets or many small ones)
#define RX_BUF_SIZE 512

typedef struct {
	uint8_t buf[RX_BUF_SIZE];
	volatile uint16_t head;
	volatile uint16_t tail;
} isler_ringbuf_t;

static isler_ringbuf_t rx_fifo;

// Helper to push data (Called from ISR)
static void rx_fifo_push(const uint8_t *data, size_t len) {
	for (size_t i = 0; i < len; i++) {
		uint16_t next = (rx_fifo.head + 1) % RX_BUF_SIZE;
		if (next != rx_fifo.tail) { // Only push if not full
			rx_fifo.buf[rx_fifo.head] = data[i];
			rx_fifo.head = next;
		}
	}
}

// Helper to pop data (Called from Python context)
static int rx_fifo_pop(void) {
	if (rx_fifo.head == rx_fifo.tail) {
		return -1; // Empty
	}
	int val = rx_fifo.buf[rx_fifo.tail];
	rx_fifo.tail = (rx_fifo.tail + 1) % RX_BUF_SIZE;
	return val;
}

static size_t rx_fifo_count(void) {
	if (rx_fifo.head >= rx_fifo.tail) {
		return rx_fifo.head - rx_fifo.tail;
	}
	return (RX_BUF_SIZE - rx_fifo.tail) + rx_fifo.head;
}

// ==========================================================================
// ISR Handler (The "Hard" Interrupt)
// ==========================================================================

// Global reference to the user's Python callback function
static mp_obj_t isler_callback_obj = mp_const_none;

void mod_isler_rx_isr(void) {
	uint8_t *frame = (uint8_t *)LLE_BUF;
	uint8_t len = frame[1] + 2;

	if (len > 0 && len < 255) {
		rx_fifo_push(frame, len);
	}

	if (isler_callback_obj != mp_const_none) {
		mp_sched_schedule(isler_callback_obj, mp_const_none);
	}
}

// ==========================================================================
// Python Methods
// ==========================================================================

// Method: iSLER.irq(handler)
static mp_obj_t fun_isler_irq(mp_obj_t handler) {
	isler_callback_obj = handler;
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(fun_isler_irq_obj, fun_isler_irq);

// Method: iSLER.any() -> Returns number of bytes available
static mp_obj_t fun_isler_any(void) {
	return MP_OBJ_NEW_SMALL_INT(rx_fifo_count());
}
static MP_DEFINE_CONST_FUN_OBJ_0(fun_isler_any_obj, fun_isler_any);

// Method: iSLER.read([len])
static mp_obj_t fun_isler_read(size_t n_args, const mp_obj_t *args) {
	size_t count = rx_fifo_count();

	if (n_args > 0) {
		size_t req = mp_obj_get_int(args[0]);
		if (req < count) count = req;
	}

	if (count == 0) {
		return mp_const_none;
	}

	vstr_t vstr;
	vstr_init_len(&vstr, count);

	for (size_t i = 0; i < count; i++) {
		vstr.buf[i] = (byte)rx_fifo_pop();
	}

	return mp_obj_new_bytes_from_vstr(&vstr);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fun_isler_read_obj, 0, 1, fun_isler_read);

// ==========================================================================
// Hardware Register Definitions
// ==========================================================================

enum { W8, W16, W32 };

typedef struct {
	qstr name;
	uintptr_t addr; // The memory address of the struct field
	uint8_t width;
} reg_entry_t;

// We assume ch32fun defines global pointers or macros for LL, RF, BB
// Example: #define BB ((BB_TypeDef *)0x40001000)
// This macro list maps the Python Name to the Hardware Address
static const reg_entry_t isler_reg_table[] = {
	// Generated macros from ch32fun
	// { MP_QSTR_BB_BB14, (uintptr_t)&BB->BB14, W32 },
	#include "ch32fun_islerregs.h"
};

// ==========================================================================
// Methods (init, tx, rx)
// ==========================================================================

static mp_obj_t fun_isler_init(mp_obj_t arg_in) {
	iSLERInit((uint8_t)mp_obj_get_int(arg_in));
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(fun_isler_init_obj, fun_isler_init);

// --------------------------------------------------------------------------
// RX Function
// Python: iSLER.rx(access_addr, channel, phy_mode)
// --------------------------------------------------------------------------
static mp_obj_t fun_isler_rx(size_t n_args, const mp_obj_t *args) {
	uint32_t access_addr = mp_obj_get_int_truncated(args[0]);
	uint8_t channel      = mp_obj_get_int(args[1]); // e.g. 37, 38, 39
	uint8_t phy_mode     = mp_obj_get_int(args[2]); // e.g. iSLER.PHY_1M

	// This is non-blocking. When a packet arrives, the ISR
	// will fill the RingBuffer and trigger the Python callback.
	iSLERRX(access_addr, channel, phy_mode);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fun_isler_rx_obj, 3, 3, fun_isler_rx);

static mp_obj_t fun_isler_tx(size_t n_args, const mp_obj_t *args) {
	mp_buffer_info_t mac;
	mp_buffer_info_t bufinfo;
	uint32_t access_addr = mp_obj_get_int_truncated(args[0]);
	mp_get_buffer_raise(args[1], &mac, MP_BUFFER_READ);
	mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
	uint8_t channel = mp_obj_get_int(args[3]);
	uint8_t phy_mode = mp_obj_get_int(args[4]);

	if (bufinfo.len > 245) {
		mp_raise_ValueError(MP_ERROR_TEXT("payload too large"));
	}
	else if (mac.len != 6) {
		mp_raise_ValueError(MP_ERROR_TEXT("MAC address is 6 bytes"));
	}

	uint8_t packet[256];
	packet[0] = 0x02; // PDU
	packet[1] = (uint8_t)bufinfo.len +6; // +6 = mac
	for(int i = 0; i < 6; i++) {
		packet[2 +i] = ((uint8_t*)mac.buf)[5 -i];
	}

	if (bufinfo.len > 0) {
		memcpy(&packet[8], bufinfo.buf, bufinfo.len);
	}

	iSLERTX(access_addr, packet, bufinfo.len +6 +2, channel, phy_mode);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fun_isler_tx_obj, 5, 5, fun_isler_tx);

static mp_obj_t fun_isler_adv(size_t n_args, const mp_obj_t *args) {
	mp_buffer_info_t mac;
	mp_buffer_info_t bufinfo;
	mp_get_buffer_raise(args[0], &mac, MP_BUFFER_READ);
	mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_READ);

	if (bufinfo.len > 250) {
		mp_raise_ValueError(MP_ERROR_TEXT("payload too large"));
	}
	else if (mac.len != 6) {
		mp_raise_ValueError(MP_ERROR_TEXT("MAC address is 6 bytes"));
	}

	uint8_t adv[256];
	adv[0] = 0x02; // PDU
	adv[1] = (uint8_t)bufinfo.len +6; // +6 = mac
	for(int i = 0; i < 6; i++) {
		adv[2 +i] = ((uint8_t*)mac.buf)[5 -i];
	}

	if (bufinfo.len > 0) {
		memcpy(&adv[8], bufinfo.buf, bufinfo.len);
	}

	iSLERTX(0x8E89BED6, adv, bufinfo.len +6 +2, 37, PHY_1M);
	iSLERTX(0x8E89BED6, adv, bufinfo.len +6 +2, 38, PHY_1M);
	iSLERTX(0x8E89BED6, adv, bufinfo.len +6 +2, 39, PHY_1M);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fun_isler_adv_obj, 2, 2, fun_isler_adv);

static const mp_rom_map_elem_t isler_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&fun_isler_init_obj) },

	{ MP_ROM_QSTR(MP_QSTR_rx),   MP_ROM_PTR(&fun_isler_rx_obj) },
	{ MP_ROM_QSTR(MP_QSTR_irq),  MP_ROM_PTR(&fun_isler_irq_obj) },
	{ MP_ROM_QSTR(MP_QSTR_any),  MP_ROM_PTR(&fun_isler_any_obj) },
	{ MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&fun_isler_read_obj) },

	{ MP_ROM_QSTR(MP_QSTR_tx),   MP_ROM_PTR(&fun_isler_tx_obj) },
	{ MP_ROM_QSTR(MP_QSTR_adv),  MP_ROM_PTR(&fun_isler_adv_obj) },

	// Constants
	{ MP_ROM_QSTR(MP_QSTR_PHY_1M), MP_ROM_INT(PHY_1M) },
	{ MP_ROM_QSTR(MP_QSTR_PHY_2M), MP_ROM_INT(PHY_2M) },
	{ MP_ROM_QSTR(MP_QSTR_PHY_S2), MP_ROM_INT(PHY_S2) },
	{ MP_ROM_QSTR(MP_QSTR_PHY_S8), MP_ROM_INT(PHY_S8) },

	// Generated macros from ch32fun
	#include "ch32fun_islerdefs.h"
};
static MP_DEFINE_CONST_DICT(isler_locals_dict, isler_locals_dict_table);

static void isler_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
	// A. Check if it is a Register Access
	const reg_entry_t *reg = NULL;
	for (size_t i = 0; i < (sizeof(isler_reg_table) / sizeof(reg_entry_t)); i++) {
		if (isler_reg_table[i].name == attr) {
			reg = &isler_reg_table[i];
			break;
		}
	}

	if (reg != NULL) {
		// Register Found!
		if (dest[0] == MP_OBJ_NULL) {
			// Load (Read)
			mp_int_t val = 0;
			if (reg->width == W32) val = *(volatile uint32_t *)reg->addr;
			else if (reg->width == W16) val = *(volatile uint16_t *)reg->addr;
			else val = *(volatile uint8_t *)reg->addr;
			dest[0] = mp_obj_new_int_from_uint(val);
		} else if (dest[1] != MP_OBJ_NULL) {
			// Store (Write)
			mp_int_t val = mp_obj_get_int_truncated(dest[1]);
			if (reg->width == W32) *(volatile uint32_t *)reg->addr = (uint32_t)val;
			else if (reg->width == W16) *(volatile uint16_t *)reg->addr = (uint16_t)val;
			else *(volatile uint8_t *)reg->addr = (uint8_t)val;
			dest[0] = MP_OBJ_NULL;
		}
		return;
	}

	// B. If NOT a register, check for Methods/Constants in the locals_dict
	// We only handle Load (method lookup), not Store/Delete for methods
	if (dest[0] == MP_OBJ_NULL) {
		mp_map_elem_t *elem = mp_map_lookup((mp_map_t*)&isler_locals_dict.map, MP_OBJ_NEW_QSTR(attr), MP_MAP_LOOKUP);
		if (elem != NULL) {
			dest[0] = elem->value;
			return;
		}
	}

	// C. Fail (AttributeError will be raised by caller if dest[0] is still NULL)
}

// NOTE: Not static
MP_DEFINE_CONST_OBJ_TYPE(
	ch32fun_isler_type,
	MP_QSTR_iSLER,
	MP_TYPE_FLAG_NONE,
	attr, isler_attr
);

// Create the Singleton Instance
const mp_obj_base_t ch32fun_isler_obj = { &ch32fun_isler_type };

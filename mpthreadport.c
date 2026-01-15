#include <stdio.h>
#include <string.h>

#include "py/mpconfig.h"
#include "py/mpstate.h"
#include "py/gc.h"
#include "py/mpthread.h"
#include "py/runtime.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#if MICROPY_PY_THREAD

// ==============================================================================
// 1. Static Configuration
// ==============================================================================

#ifndef MP_THREAD_MAX
#define MP_THREAD_MAX  4 
#endif

// ==============================================================================
// 2. The Static Thread Slot Registry
// ==============================================================================

typedef struct _mp_thread_slot_t {
	int in_use;                 
	TaskHandle_t task;          

	// MicroPython State (Typedef from mpstate.h)
	mp_state_thread_t *state;   
	
	void *stack_top;            
	size_t stack_len;           

	void *(*entry)(void *);
	void *arg;

} mp_thread_slot_t;

static mp_thread_slot_t thread_pool[MP_THREAD_MAX];
static SemaphoreHandle_t pool_mutex = NULL; 

mp_thread_slot_t* get_current_slot(void) {
	TaskHandle_t current = xTaskGetCurrentTaskHandle();
	
	if (pool_mutex) xSemaphoreTake(pool_mutex, portMAX_DELAY);
	
	for (int i = 0; i < MP_THREAD_MAX; i++) {
		if (thread_pool[i].in_use && thread_pool[i].task == current) {
			if (pool_mutex) xSemaphoreGive(pool_mutex);
			return &thread_pool[i];
		}
	}
	
	if (pool_mutex) xSemaphoreGive(pool_mutex);
	return NULL;
}

// ==============================================================================
// 3. MicroPython Port API
// ==============================================================================

mp_uint_t mp_thread_get_id(void) {
	return (mp_uint_t)xTaskGetCurrentTaskHandle();
}

void mp_thread_init(void *stack, uint32_t stack_len) {
	memset(thread_pool, 0, sizeof(thread_pool));
	pool_mutex = xSemaphoreCreateMutex();

	mp_thread_set_state(&mp_state_ctx.thread);
	
	xSemaphoreTake(pool_mutex, portMAX_DELAY);
	thread_pool[0].in_use = 1;
	thread_pool[0].task = xTaskGetCurrentTaskHandle();
	thread_pool[0].state = &mp_state_ctx.thread;
	thread_pool[0].stack_len = stack_len;
	xSemaphoreGive(pool_mutex);
}

mp_state_thread_t *mp_thread_get_state(void) {
	mp_thread_slot_t *slot = get_current_slot();
	return (slot != NULL) ? slot->state : NULL;
}

void mp_thread_set_state(mp_state_thread_t *state) {
	mp_thread_slot_t *slot = get_current_slot();
	if (slot != NULL) {
		slot->state = state;
	}
}

void mp_thread_start(void) {
	xSemaphoreTake(pool_mutex, portMAX_DELAY);
}

void mp_thread_finish(void) {
	xSemaphoreGive(pool_mutex);
}

// --------------------------------------------------------------------------
// Thread Entry Trampoline
// --------------------------------------------------------------------------
void freertos_entry_trampoline(void *ptr) {
	mp_thread_slot_t *slot = (mp_thread_slot_t *)ptr;
	
	volatile int stack_dummy;
	void *stack_top = (void*)&stack_dummy;

	xSemaphoreTake(pool_mutex, portMAX_DELAY);
	slot->stack_top = stack_top;
	slot->task = xTaskGetCurrentTaskHandle(); 
	xSemaphoreGive(pool_mutex);

	mp_thread_set_state(slot->state);

	slot->entry(slot->arg);

	xSemaphoreTake(pool_mutex, portMAX_DELAY);
	slot->in_use = 0;
	slot->task = NULL;
	xSemaphoreGive(pool_mutex);

	vTaskDelete(NULL);
	for (;;);
}

// --------------------------------------------------------------------------
// Create Thread
// --------------------------------------------------------------------------

mp_uint_t mp_thread_create(void *(*entry)(void *), void *arg, size_t *stack_size) {
	if (*stack_size == 0) {
		*stack_size = 4096; 
	}

	mp_state_thread_t *ts = m_new_obj(mp_state_thread_t);
	
	mp_thread_slot_t *slot = NULL;
	
	xSemaphoreTake(pool_mutex, portMAX_DELAY);
	for (int i = 0; i < MP_THREAD_MAX; i++) {
		if (!thread_pool[i].in_use) {
			slot = &thread_pool[i];
			slot->in_use = 1; 
			break;
		}
	}
	xSemaphoreGive(pool_mutex);

	if (slot == NULL) {
		mp_raise_msg(&mp_type_OSError, "no free thread slots");
		return 0;
	}

	slot->entry = entry;
	slot->arg = arg;
	slot->state = ts;
	slot->stack_len = *stack_size;
	
	TaskHandle_t id;
	BaseType_t res = xTaskCreate(freertos_entry_trampoline, "MPThread", 
								 *stack_size / sizeof(StackType_t), 
								 (void*)slot, 
								 2, &id);

	if (res != pdPASS) {
		xSemaphoreTake(pool_mutex, portMAX_DELAY);
		slot->in_use = 0;
		xSemaphoreGive(pool_mutex);
		mp_raise_msg(&mp_type_OSError, "can't create thread");
		return 0;
	}

	// Return the TaskHandle cast to integer ID
	return (mp_uint_t)id;
}

// ==============================================================================
// 4. Mutex Implementation 
// ==============================================================================

void mp_thread_mutex_init(mp_thread_mutex_t *mutex) {
	mutex->handle = xSemaphoreCreateMutex();
}

int mp_thread_mutex_lock(mp_thread_mutex_t *mutex, int wait) {
	return (xSemaphoreTake(mutex->handle, wait ? portMAX_DELAY : 0) == pdTRUE);
}

void mp_thread_mutex_unlock(mp_thread_mutex_t *mutex) {
	xSemaphoreGive(mutex->handle);
}

void mp_thread_deinit(void) {
}

#endif // MICROPY_PY_THREAD

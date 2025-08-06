#ifndef INC_TASK_LED_H_
#define INC_TASK_LED_H_

#include "cmsis_os.h"
#include <stdbool.h>

typedef enum {
	LED_EVENT_RED, LED_EVENT_GREEN, LED_EVENT_BLUE
} led_event_type_t;

typedef void (*callback_t)(void *context);

typedef struct {
	led_event_type_t type;
	callback_t callback_process_completed;
	void *callback_context;
} led_event_t;

typedef struct {
	QueueHandle_t queue;
} led_t;

#define NUM_LEDS 3

/* Declaraciones de las tareas para los LEDs */
void process_led_event(led_t *led);
bool add_led_event_to_queue(led_t *leds, led_event_type_t event_type, led_event_t *event);

#endif /* INC_TASK_LED_H_ */

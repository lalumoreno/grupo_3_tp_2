/*
 * task_led.h
 *
 *  Created on: Jul 5, 2025
 *      Author: HP VICTUS
 */

#ifndef INC_TASK_LED_H_
#define INC_TASK_LED_H_

#include "cmsis_os.h"

typedef enum {
    LED_EVENT_RED,
    LED_EVENT_GREEN,
    LED_EVENT_BLUE
} led_event_type_t;

typedef struct {
    led_event_type_t type;
} led_event_t;


typedef struct {
    QueueHandle_t queue;
} led_t;

/* Declaraciones de las tareas para los LEDs */
void task_led(void *argument);

#endif /* INC_TASK_LED_H_ */

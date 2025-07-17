/*
 * app.h
 *
 *  Created on: Jul 5, 2025
 *      Author: HP VICTUS
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "cmsis_os.h"
#include "task_button.h"  // Para button_type_t

/* Recursos globales */
extern QueueHandle_t button_event_queue;
extern SemaphoreHandle_t sem_led_red;
extern SemaphoreHandle_t sem_led_green;
extern SemaphoreHandle_t sem_led_blue;

/* Inicialización del sistema */
void app_init(void);

#endif /* INC_APP_H_ */

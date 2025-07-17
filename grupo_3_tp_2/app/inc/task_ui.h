/*
 * task_ui.h
 *
 *  Created on: Jul 5, 2025
 *      Author: HP VICTUS
 */

#ifndef INC_TASK_UI_H_
#define INC_TASK_UI_H_

#include "cmsis_os.h"
#include "task_button.h"  // para button_type_t

/* Declaración de la tarea de Interfaz de Usuario */
void task_ui(void *argument);

#endif /* INC_TASK_UI_H_ */

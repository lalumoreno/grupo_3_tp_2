#ifndef INC_TASK_UI_H_
#define INC_TASK_UI_H_

#include "task_button.h"  // para button_type_t
#include <stdbool.h>

/* Declaración de la tarea de Interfaz de Usuario */
void init_ui_queue();
void task_ui(void *argument);
bool add_button_event_to_queue(button_event_t *event);

#endif /* INC_TASK_UI_H_ */

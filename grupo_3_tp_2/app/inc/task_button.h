#ifndef INC_TASK_BUTTON_H_
#define INC_TASK_BUTTON_H_

#include "cmsis_os.h"

typedef enum
{
  BUTTON_TYPE_NONE = 0,
  BUTTON_TYPE_PULSE,
  BUTTON_TYPE_SHORT,
  BUTTON_TYPE_LONG,
} button_type_t;

typedef void (*callback_t)(void *context);

typedef struct
{
  button_type_t type;
  uint32_t duration;
  callback_t callback_process_completed;
  void *callback_context;
} button_event_t;

void task_button(void *argument);

#endif /* INC_TASK_BUTTON_H_ */

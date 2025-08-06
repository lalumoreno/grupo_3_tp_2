#include "main.h"
#include "cmsis_os.h"
#include "task_button.h"
#include "task_ui.h"
#include "task_led.h"
#include "task_uart.h"
#include "board.h"

#define TASK_BUTTON_STACK_SIZE 128
#define TASK_BUTTON_PRIORITY (tskIDLE_PRIORITY + 1)
#define UART_QUEUE_LENGTH 20

/* Prototipos de funciones */

static void init_task_button(void);

static void init_task_button(void)
{
	BaseType_t status = xTaskCreate(task_button, "task_button", TASK_BUTTON_STACK_SIZE, NULL, TASK_BUTTON_PRIORITY, NULL);
	configASSERT(status == pdPASS);
	if (status != pdPASS)
	{
		log_uart("Error: No se pudo crear la tarea Button\r\n");
	}
	else
	{
		log_uart("Tarea Button creada correctamente\r\n");
	}
}

void app_init(void)
{
	/* Crear tareas del sistema */
	init_task_uart();
	init_task_button();

	/* Enviar mensaje por UART */
	log_uart("APP - App init\r\n");
#ifdef _F429ZI_
	log_uart("APP - Board F429ZI\r\n");
#endif
}

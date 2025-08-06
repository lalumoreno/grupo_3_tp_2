#include "main.h"
#include "cmsis_os.h"
#include "task_button.h"
#include "task_ui.h"
#include "task_led.h"
#include "task_uart.h"
#include "board.h"

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

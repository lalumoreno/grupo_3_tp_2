#include "main.h"
#include "cmsis_os.h"
#include "task_button.h"
#include "task_ui.h"
#include "task_led.h"
#include "task_uart.h"
#include "board.h"

void app_init(void) {

	/* Crear tareas del sistema */
	BaseType_t status;
	status = xTaskCreate(task_uart, "task_uart", 128, NULL,
	tskIDLE_PRIORITY + 1, NULL);
	configASSERT(status == pdPASS);
log_uart("APP - test init\r\n");
/*
	status = xTaskCreate(task_button, "task_button", 128, NULL,
	tskIDLE_PRIORITY + 2, NULL);
	configASSERT(status == pdPASS);
*/
	/* Enviar mensaje por UART */
	log_uart("APP - App init\r\n");

#ifdef _F429ZI_
	log_uart("APP - Board F429ZI\r\n");
#endif

}

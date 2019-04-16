#include <stdio.h>

#include "os_inc.h"

#include "stm32f10x.h"

#include "bsp.h"

#include "mid_dbg.h"

#include "app_inc.h"



/* The check task uses the sprintf function so requires a little more stack. */
#define mainLED_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE + 50 )
	
static TaskHandle_t task_handle[TASK_HANDLE_ALL];

 TaskHandle_t main_get_task_handle(unsigned char id)
{
	return task_handle[id];
}

static void hardware_init(void)
{
	bsp_init();
	mid_dbg_init();
	app_led_init();
	app_data_init();
}


/* The time between cycles of the 'check' task. */
int main( void )
{
	static char cmd_prefix[20];
	hardware_init();
	dbg_string("------Glow tube clock!------\r\n");

	sprintf(cmd_prefix, "%s-%d.%d.%d ", "clock", PRJ_VER_MAJOR, PRJ_VER_MINOR, PRJ_VER_REVISION);
	app_cli_init(tskIDLE_PRIORITY + 2, cmd_prefix, &task_handle[TASK_HANDLE_CLI]);

	/* establish lwip, create udp and tcp server */
	LwIP_Init();

    #if (1)
	xTaskCreate((pdTASK_CODE)app_display_task, "display", 280, NULL, 2, &task_handle[TASK_HANDLE_DISPLAY]);
	xTaskCreate((pdTASK_CODE)app_th_task, "sht10", 280, NULL, 2, &task_handle[TASK_HANDLE_SHT10]);
	xTaskCreate((pdTASK_CODE)app_time_task, "clock", 280, NULL, 2, &task_handle[TASK_HANDLE_DS3231]);
	xTaskCreate((pdTASK_CODE)app_buzzer_task, "buzzer", 290, NULL, 2, &task_handle[TASK_HANDLE_BUZ]);
	xTaskCreate((pdTASK_CODE)app_led_task_blink, "led-RGB", 200, NULL, tskIDLE_PRIORITY + 1, &task_handle[TASK_HANDLE_LED]);
	xTaskCreate((pdTASK_CODE)app_data_store_task, "storage", 200, NULL, 2, &task_handle[TASK_HANDLE_DATA]);
    #endif
    
	/* St0art the scheduler. */
	vTaskStartScheduler();

	return 0;
}



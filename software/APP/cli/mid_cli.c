#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Utils includes. */
#include "hal_cli.h"
#include "mid_cli.h"

typedef struct _list_command_t
{
	const struct _command_t *module;	/**< ab*/
	struct _list_command_t *next;
} list_command_t;

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE		256
#define	cmdMAX_OUTPUT_SIZE		256


/* DEL acts as a backspace. */
#define cmdASCII_DEL		( 0x7F )
#define cmdASCII_BS			'\b'
#define cmdASCII_NEWLINE	'\n'
#define cmdASCII_HEADLINE	'\r'
#define cmdASCII_STRINGEND	'\0'
#define cmdASCII_SPACE		' '
#define	cmdASCII_TILDE		'~'

enum passwd_state {
	PASSWD_INCORRECT = 0,
	PASSWD_CORRECT = 1,
};

static int8_t mid_cli_string_split(char **dest, const char *commandString);
static void mid_cli_console_task(void *pvParameters);
static BaseType_t mid_cli_parse_command(const char * const input, char *dest, size_t len);


static const char * const passwd = "jhg";
static const char * const input_passwd_msg = "Input password: ";
static const char * const incorrect_passwd_msg = "Password incorrect!\r\n";
static const char * def_prefix = "<Terminal> ";
static const char * prefix = NULL;//"<Terminal> ";
static const char * const pcNewLine = "\r\n";
static const char * const backspace = " \b";

build_var(help, "Lists all the registered commands.\r\n", 0);
static struct _list_command_t cmd_list_head = 
{
	&help,
	NULL,
};

static char vars[cmdMAX_VARS_SIZE][cmdMAX_STRING_SIZE];
static char *argv[cmdMAX_VARS_SIZE];
static char cOutputString[cmdMAX_OUTPUT_SIZE];
static char cInputString[cmdMAX_INPUT_SIZE];

struct _list_command_t *mid_cli_cmd_list_head(void)
{
    return &cmd_list_head;
}

/*
 * ����ע�ắ��
 */
BaseType_t mid_cli_register(const struct _command_t * const p)
{
    static struct _list_command_t *pxLastCommandInList = &cmd_list_head;
    static struct _list_command_t *pxNewListItem;
    BaseType_t xReturn = pdFAIL;

	/* Check the parameter is not NULL. */
	configASSERT(p);

	/* ��������ڵ����Ӹ������� */
	pxNewListItem = (list_command_t *) pvPortMalloc(sizeof(list_command_t));
	configASSERT(pxNewListItem);

	if( pxNewListItem != NULL )
	{
		taskENTER_CRITICAL();
		{
			/* ����µĳ�Ա������ */
			pxNewListItem->module = p;

			/* ����β�ڵ��־ */
			pxNewListItem->next = NULL;

			/* �����³�Ա */
			pxLastCommandInList->next = pxNewListItem;

			/* ��¼��ǰ�ڵ㣬�ȴ���һ���ڵ㱻���� */
			pxLastCommandInList = pxNewListItem;
		}
		taskEXIT_CRITICAL();

		xReturn = pdPASS;
	}

	return xReturn;
}

void mid_cli_init(unsigned short usStackSize, UBaseType_t uxPriority, char *t, TaskHandle_t *h)
{
	char i;

	for(i = 0; i < cmdMAX_VARS_SIZE; i ++)
	{
		argv[i] = vars[i];
	}
	if(t != NULL)
		prefix = t;
	else
		prefix = def_prefix;
	/* Create that task that handles the console itself. */
	xTaskCreate(mid_cli_console_task,		/* The task that implements the command console. */
				"cmd",				/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
				usStackSize,				/* The size of the stack allocated to the task. */
				NULL,						/* The parameter is not used, so NULL is passed. */
				uxPriority,					/* The priority allocated to the task. */
				h);							/* A handle is not required, so just pass NULL. */
}

static void mid_cli_console_task(void *parame)
{
	static enum passwd_state permission = PASSWD_INCORRECT;
	unsigned char inputIndex = 0;
	unsigned char status = 0;
	signed char inputChar;

	for(;;)
	{
		/* ������� */
		if(status == 0)
		{
			/* �ȴ��ն����� */
			while(hal_cli_data_rx(&inputChar, 0) != pdTRUE) ;
			
			/* �������ݻ��� */
			if(inputChar != cmdASCII_BS || inputIndex)
				hal_cli_data_tx(&inputChar, sizeof(inputChar));
			
			if(inputChar == cmdASCII_NEWLINE || inputChar == cmdASCII_HEADLINE)
			{
				status = 1;	/* ����������ϣ�����Ȩ���ж� */
			}
			else			/* ����׶Σ���֧��backspace��ɾ���� */
			{
				if(inputChar == cmdASCII_BS)
				{
					if(inputIndex > 0)
					{
						inputIndex --;
						cInputString[ inputIndex ] = cmdASCII_STRINGEND;
						hal_cli_data_tx((signed char *)backspace, strlen(backspace));
					}
				}
				else if((inputChar >= cmdASCII_SPACE) && (inputChar <= cmdASCII_TILDE))
				{
					if( inputIndex < cmdMAX_INPUT_SIZE)
					{
						cInputString[inputIndex] = inputChar;
						inputIndex ++;
					}
				}
			}
		}
		/* �����ɣ�Ȩ���ж� */
		if(status == 1)	
		{
			hal_cli_data_tx((signed char *)pcNewLine, strlen(pcNewLine));
			if(permission == PASSWD_INCORRECT)
				status = 2;
			else
				status = 3;
		}
		/* ��ȫ��֤ */
		if(status == 2)
		{
			if(strcmp(passwd, cInputString))
			{
				if(inputIndex)
				{
					hal_cli_data_tx((signed char *)incorrect_passwd_msg, strlen(incorrect_passwd_msg));
				}
				hal_cli_data_tx((signed char *)input_passwd_msg, strlen(input_passwd_msg));
				status = 0;
			}
			else
			{
				permission = PASSWD_CORRECT;
				status = 3;
			}
			memset(cInputString, 0, cmdMAX_INPUT_SIZE);
			inputIndex = 0;
		}
		/* ������� */
		if(status == 3)
		{
			if(inputIndex != 0)
			{
				BaseType_t xReturned;
				
				do
				{
					memset(cOutputString, '\0', strlen(cOutputString));
					xReturned = mid_cli_parse_command(cInputString, cOutputString, cmdMAX_OUTPUT_SIZE);
					hal_cli_data_tx((signed char *)cOutputString, strlen(cOutputString));
				} while(xReturned != pdFALSE);
				memset(cInputString, 0, cmdMAX_INPUT_SIZE);
				memset(vars, 0, sizeof(vars));
				inputIndex = 0;
			}
			hal_cli_data_tx((signed char *)prefix, strlen(prefix));
			status = 0;
		}
	}
}

static BaseType_t mid_cli_parse_command(const char * const input, char *dest, size_t len)
{
	static const struct _list_command_t *cmd = NULL;
	BaseType_t xReturn = pdTRUE;
	
	const char *cmdString;
	size_t cmdStringLen;
	const static char log1[] = " not recognised. Input 'help' to view available commands.\r\n";

	
	if(cmd == NULL)
	{
		for(cmd = &cmd_list_head; cmd != NULL; cmd = cmd->next)
		{
			cmdString = cmd->module->command;
			cmdStringLen = strlen(cmdString);

			if((input[cmdStringLen] == cmdASCII_SPACE) || (input[cmdStringLen] == 0))
			{
				if(strncmp(input, cmdString, cmdStringLen) == 0)
				{
					/* ����������������������趨�ĸ���������Ϊ������Ч */
					if(mid_cli_string_split(argv, input) != cmd->module->expect_parame_num)
					{
						xReturn = pdFALSE;
					}
					break;
				}
			}
		}
	}
    
	if((cmd != NULL) && (xReturn == pdFALSE))
	{
		/* ���ֵ����µ��������������������������ƥ�� */
		sprintf(dest, "  '%s'%s", input, log1);
		cmd = NULL;
	}
	else if(cmd != NULL)
	{
		memset(dest, cmdASCII_STRINGEND, sizeof(char) * cmdMAX_OUTPUT_SIZE);
		/* ִ��ע������Ļص����� */
		xReturn = cmd->module->handle(dest, argv, cmd->module->help_info);

		/* �������ʱ pdFALSE�����������������Ϣ����ϣ�׼��������һ������ */
		if(xReturn == pdFALSE)
		{
			cmd = NULL;
		}
	}
	else
	{
		/* ����Ϊ�գ�û�з����µ����������� */
        if(*input != 0)
            sprintf(dest, "  '%s'%s", input, log1);
		else
			memset(dest, cmdASCII_STRINGEND, sizeof(char) * cmdMAX_OUTPUT_SIZE);
		xReturn = pdFALSE;
	}
	
	return xReturn;
}

static int8_t mid_cli_string_split(char **dest, const char *commandString)
{
	int8_t num = 0, index = 0;
	BaseType_t wasSpace = pdFALSE;

	while(*commandString != cmdASCII_STRINGEND)
	{
		if((*commandString) == cmdASCII_SPACE)
		{
			if(wasSpace == pdFALSE)
			{
				dest[num][index] = cmdASCII_STRINGEND;
				num++;
				index = 0;
				wasSpace = pdTRUE;
			}
		}
		else
		{
			dest[num][index++] = *commandString;
			wasSpace = pdFALSE;
		}

		commandString++;
	}


	return num;
}

static BaseType_t help_main(char *dest, argv_attribute argv, const char * const help_info)
{
	static struct _list_command_t *cmd = NULL;
	
	( void ) help_info;

	configASSERT( dest );

	if(cmd == NULL)
	{
		cmd = mid_cli_cmd_list_head();
	}
	else
	{
        strcat(dest, "\t");
        strcat(dest, cmd->module->command);
        strcat(dest, ":\t");
        strcat(dest, cmd->module->help_info);
	}
	cmd = cmd->next;
	if(cmd == NULL)
		return pdFALSE;		/* ������Ϣ������� */
	else
		return pdTRUE;
}


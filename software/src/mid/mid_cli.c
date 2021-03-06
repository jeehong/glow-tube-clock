#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Utils includes. */
#include "hal_cli.h"
#include "mid_cli.h"

typedef struct _list_command_t
{
	const struct _command_t *module;	/**< ab*/
	struct _list_command_t *next;
} list_command_t;

#define CLI_VERSION_MAJOR		(0)		/* 主版本号 */
#define CLI_VERSION_MINOR		(1)		/* 次版本号 */


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

#ifdef CLI_SUPPORT_PASSWD
static const char * const passwd = "jhg";
static const char * const input_passwd_msg = "Input password: ";
static const char * const incorrect_passwd_msg = "Password incorrect!\r\n";
#endif
static const char * def_prefix = "Terminal ";
static const char * prefix = NULL;
static const char * const pcNewLine = "\r\n";
static const char * const backspace = " \b";
static const char * err_remind = " not recognised. Input 'help' to view available commands.\r\n";

build_var(help, "Lists all the registered commands.", 0);
static struct _list_command_t cmd_list_head = 
{
	&help,
	NULL,
};

static char vars[cmdMAX_VARS_SIZE][cmdMAX_STRING_SIZE];
static char *xargv[cmdMAX_VARS_SIZE];
static char cOutputString[cmdMAX_OUTPUT_SIZE];
static char cInputString[cmdMAX_INPUT_SIZE];

/*
 * 命令注册函数
 */
BaseType_t mid_cli_register(const struct _command_t * const p)
{
    static struct _list_command_t *pxLastCommandInList = &cmd_list_head;
    static struct _list_command_t *pxNewListItem;
    BaseType_t xReturn = pdFAIL;

	/* Check the parameter is not NULL. */
	configASSERT(p);

	/* 创建链表节点链接给新命令 */
	pxNewListItem = (list_command_t *) cli_malloc(sizeof(list_command_t));
	configASSERT(pxNewListItem);

	if( pxNewListItem != NULL )
	{
		taskENTER_CRITICAL();
		{
			/* 添加新的成员到链表 */
			pxNewListItem->module = p;

			/* 设置尾节点标志 */
			pxNewListItem->next = NULL;

			/* 链接新成员 */
			pxLastCommandInList->next = pxNewListItem;

			/* 记录当前节点，等待下一个节点被链接 */
			pxLastCommandInList = pxNewListItem;
		}
		taskEXIT_CRITICAL();

		xReturn = pdPASS;
	}

	return xReturn;
}

void mid_cli_init(U16 usStackSize, UBaseType_t uxPriority, char *t, TaskHandle_t *h)
{
	U8 i;

	for(i = 0; i < cmdMAX_VARS_SIZE; i ++)
	{
		xargv[i] = vars[i];
	}
	if(t != NULL)
		prefix = t;
	else
		prefix = def_prefix;
	/* Create that task that handles the console itself. */
	xTaskCreate(mid_cli_console_task,	/* The task that implements the command console. */
				"cmd",					/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
				usStackSize,			/* The size of the stack allocated to the task. */
				NULL,					/* The parameter is not used, so NULL is passed. */
				uxPriority,				/* The priority allocated to the task. */
				h);						/* A handle is not required, so just pass NULL. */
}

static void mid_cli_console_task(void *parame)
{
	#ifdef CLI_SUPPORT_PASSWD
	static enum passwd_state permission = PASSWD_INCORRECT;
	#endif
	U8 inputIndex = 0;
	U8 status = 0;
	char inputChar;

	for(;;)
	{
		/* 正在组包 */
		if(status == 0)
		{
			/* 等待终端输入 */
			while(hal_cli_data_rx(&inputChar, 0) != pdTRUE) ;
			
			/* 输入数据回显 */
			if(inputChar != cmdASCII_BS || inputIndex)
			{
				#ifdef CLI_SUPPORT_PASSWD
				if(permission != PASSWD_INCORRECT)
				{
					hal_cli_data_tx(&inputChar, sizeof(inputChar));
				}
				#else
				hal_cli_data_tx(&inputChar, sizeof(inputChar));
				#endif
			}
			
			if(inputChar == cmdASCII_NEWLINE || inputChar == cmdASCII_HEADLINE)
			{
				#ifdef CLI_SUPPORT_PASSWD
				status = 1; /* 整包接收完毕，进入权限判定 */
				#else
				status = 3; /* 直接解析命令，无需权限判定 */
				#endif
				hal_cli_data_tx((char *)pcNewLine, strlen(pcNewLine));
			}
			else			/* 组包阶段，且支持backspace回删功能 */
			{
				if(inputChar == cmdASCII_BS)
				{
					if(inputIndex > 0)
					{
						inputIndex --;
						cInputString[ inputIndex ] = cmdASCII_STRINGEND;
						hal_cli_data_tx((char *)backspace, strlen(backspace));
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
		#ifdef CLI_SUPPORT_PASSWD
		/* 组包完成，权限判定 */
		if(status == 1)	
		{
			if(permission == PASSWD_INCORRECT)
				status = 2;
			else
				status = 3;
		}
		/* 安全认证 */
		if(status == 2)
		{
			if(strcmp(passwd, cInputString))
			{
				if(inputIndex)
				{
					hal_cli_data_tx((char *)incorrect_passwd_msg, strlen(incorrect_passwd_msg));
				}
				hal_cli_data_tx((char *)input_passwd_msg, strlen(input_passwd_msg));
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
		#endif
		/* 命令解析 */
		if(status == 3)
		{
			if(inputIndex != 0)
			{
				BaseType_t xReturned;
				
				do
				{
					memset(cOutputString, '\0', strlen(cOutputString));
					xReturned = mid_cli_parse_command(cInputString, cOutputString, cmdMAX_OUTPUT_SIZE);
					hal_cli_data_tx((char *)cOutputString, strlen(cOutputString));
				} while(xReturned != pdFALSE);
				memset(cInputString, 0, cmdMAX_INPUT_SIZE);
				memset(vars, 0, sizeof(vars));
				inputIndex = 0;
			}
			hal_cli_data_tx((char *)prefix, strlen(prefix));
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
					/* 如果输入的命令参数不符合设定的个数，则认为命令无效 */
					if(mid_cli_string_split(xargv, input) != cmd->module->expect_parame_num)
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
		/* 发现到有新的数据流，但是命令参数个数不匹配 */
		sprintf(dest, "  '%s'%s", input, err_remind);
		cmd = NULL;
	}
	else if(cmd != NULL)
	{
		memset(dest, cmdASCII_STRINGEND, sizeof(U8) * cmdMAX_OUTPUT_SIZE);
		/* 执行注册命令的回调函数 */
		xReturn = cmd->module->handle(dest, xargv, cmd->module->help_info);

		/* 如果返回时 pdFALSE，表明命令函数返回信息已完毕，准备解析下一个命令 */
		if(xReturn == pdFALSE)
		{
			cmd = NULL;
		}
	}
	else
	{
		/* 命令为空，没有发现新的数据流输入 */
        if(*input != 0)
            sprintf(dest, "  '%s'%s", input, err_remind);
		else
			memset(dest, cmdASCII_STRINGEND, sizeof(U8) * cmdMAX_OUTPUT_SIZE);
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
		if(*commandString == cmdASCII_SPACE
			&& wasSpace == pdFALSE)
		{
			dest[num][index] = cmdASCII_STRINGEND;
			num++;
			index = 0;
			wasSpace = pdTRUE;
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

cmd_handle(help)
{
	static struct _list_command_t *cmd = NULL;
	
	( void ) help_info;

	configASSERT( dest );

	if(cmd == NULL)
	{
		cmd = &cmd_list_head;
		sprintf(dest, "\033[32;49;7m        COMMAND HELP                        VER:%d.%d      \033[32;49;0m\r\n", CLI_VERSION_MAJOR, CLI_VERSION_MINOR);
	}
	else
	{
	    strcat(dest, "\t\033[32;49;1m");	    
		strcat(dest, cmd->module->command);	    
		strcat(dest, "\033[32;49;0m:\t");
        strcat(dest, cmd->module->help_info);
		strcat(dest, pcNewLine);
		cmd = cmd->next;
	}

	if(cmd == NULL)
		return pdFALSE;		/* 所有信息发送完毕 */
	else
		return pdTRUE;
}


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

/* If the application writer needs to place the buffer used by the CLI at a
fixed address then set configAPPLICATION_PROVIDES_out_buffer to 1 in
FreeRTOSConfig.h, then declare an array with the following name and size in 
one of the application files:
	char out_buffer[ CLI_STRING_MAX_OUTPUT_SIZE ];
*/
#ifndef configAPPLICATION_PROVIDES_out_buffer
	#define configAPPLICATION_PROVIDES_out_buffer 0
#endif

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE		256

#define	CLI_STRING_MAX_OUTPUT_SIZE	2048
/* Dimentions a buffer to be used by the UART driver, if the UART driver uses a
buffer at all. */
#define cmdQUEUE_LENGTH			256

/* DEL acts as a backspace. */
#define cmdASCII_DEL		( 0x7F )

/* The maximum time to wait for the mutex that guards the UART to become
available. */
#define cmdMAX_MUTEX_WAIT		pdMS_TO_TICKS( 300 )

#ifndef configCLI_BAUD_RATE
	#define configCLI_BAUD_RATE	115200
#endif

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


/*
 * Return the number of parameters that follow the command name.
 */
static int8_t get_num_of_parame(const char *commandString);

static enum passwd_state permission = PASSWD_INCORRECT;


/* The definition of the "help" command.  This command is always at the front
of the list of registered commands. */
build_var(help, "Lists all the registered commands.\r\n", 0);

/* The definition of the list of commands.  Commands that are registered are
added to this list. */
static struct cli_module_list_t cmd_list_head = 
{
	&help,
	NULL,
};

/* A buffer into which command outputs can be written is declared here, rather
than in the command console implementation, to allow multiple command consoles
to share the same buffer.  For example, an application may allow access to the
command interpreter by UART and by Ethernet.  Sharing a buffer is done purely
to save RAM.  Note, however, that the command console itself is not re-entrant,
so only one command interpreter interface can be used at any one time.  For that
reason, no attempt at providing mutual exclusion to the out_buffer array is
attempted.

configAPPLICATION_PROVIDES_out_buffer is provided to allow the application
writer to provide their own out_buffer declaration in cases where the
buffer needs to be placed at a fixed address (rather than by the linker). */
#if( configAPPLICATION_PROVIDES_out_buffer == 0 )
	static char out_buffer[CLI_STRING_MAX_OUTPUT_SIZE];
#else
	extern char out_buffer[CLI_STRING_MAX_OUTPUT_SIZE];
#endif


/*-----------------------------------------------------------*/
__inline struct cli_module_list_t *mid_cli_cmd_list_head(void)
{
    return &cmd_list_head;
}

BaseType_t mid_cli_module_register(const struct cli_module_t * const p)
{
    static struct cli_module_list_t *pxLastCommandInList = &cmd_list_head;
    static struct cli_module_list_t *pxNewListItem;
    BaseType_t xReturn = pdFAIL;

	/* Check the parameter is not NULL. */
	configASSERT(p);

	/* Create a new list item that will reference the command being registered. */
	pxNewListItem = (CLI_MODULE_LIST_t *) pvPortMalloc(sizeof(CLI_MODULE_LIST_t));
	configASSERT(pxNewListItem);

	if( pxNewListItem != NULL )
	{
		taskENTER_CRITICAL();
		{
			/* Reference the command being registered from the newly created
			list item. */
			pxNewListItem->module = p;

			/* The new list item will get added to the end of the list, so
			next has nowhere to point. */
			pxNewListItem->next = NULL;

			/* Add the newly created list item to the end of the already existing
			list. */
			pxLastCommandInList->next = pxNewListItem;

			/* Set the end of list marker to the new list item. */
			pxLastCommandInList = pxNewListItem;
		}
		taskEXIT_CRITICAL();

		xReturn = pdPASS;
	}

	return xReturn;
}

/*-----------------------------------------------------------*/


/*
 * The task that implements the command console processing.
 */
static void mid_cli_console_task(void *pvParameters);

/*-----------------------------------------------------------*/

/* Const messages output by the command console. */
static const char * const passwd = "jhg";
static const char * const input_passwd_msg = "Input password:";
static const char * const incorrect_passwd_msg = "[!]incorrect password for";
static const char * const user_name = "Terminal";
static const char * const pcNewLine = "\r\n";
static const char * const backspace = " \b";


/* Used to guard access to the UART in case messages are sent to the UART from
more than one task. */
static SemaphoreHandle_t xTxMutex = NULL;

/*-----------------------------------------------------------*/

void mid_cli_init(unsigned short usStackSize, UBaseType_t uxPriority, TaskHandle_t *h)
{
	/* Create the semaphore used to access the UART Tx. */
	xTxMutex = xSemaphoreCreateMutex();
	configASSERT(xTxMutex);

	/* Create that task that handles the console itself. */
	xTaskCreate(mid_cli_console_task,		/* The task that implements the command console. */
				"cli",				/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
				usStackSize,				/* The size of the stack allocated to the task. */
				NULL,						/* The parameter is not used, so NULL is passed. */
				uxPriority,					/* The priority allocated to the task. */
				h);							/* A handle is not required, so just pass NULL. */
}
/*-----------------------------------------------------------*/

static void mid_cli_console_task(void *parame)
{
	signed char rx_char;
	unsigned char in_index = 0;
	char *pcOutputString;
	char in_string[cmdMAX_INPUT_SIZE];
	BaseType_t xReturned;

	(void)parame;

	/* Obtain the address of the output buffer.  Note there is no mutual
	exclusion on this buffer as it is assumed only one command console interface
	will be used at any one time. */
	pcOutputString = mid_cli_output_buffer();

	/* Send the welcome message. */

	for(;;)
	{
		/* Wait for the next character.  The while loop is used in case
		INCLUDE_vTaskSuspend is not set to 1 - in which case portMAX_DELAY will
		be a genuine block time rather than an infinite block time. */
		hal_cli_data_rx(&rx_char, 1);

		/* Ensure exclusive access to the UART Tx. */
		if(xSemaphoreTake(xTxMutex, cmdMAX_MUTEX_WAIT ) == pdPASS)
		{
			/* Echo the character back. */
			if(permission == PASSWD_CORRECT)
			{
				if(rx_char == cmdASCII_BS && in_index)
					hal_cli_data_tx(&rx_char, sizeof(rx_char));
				else if(rx_char != cmdASCII_BS)
					hal_cli_data_tx(&rx_char, sizeof(rx_char));
			}


			/* Was it the end of the line? */
			if(rx_char == cmdASCII_NEWLINE || rx_char == cmdASCII_HEADLINE)
			{
				/* Just to space the output from the input. */
				hal_cli_data_tx((signed char *)pcNewLine, strlen(pcNewLine));
				if(permission == PASSWD_INCORRECT)
				{
					if(strcmp(passwd, in_string))
					{
						if(in_index)
						{
							sprintf((signed char *)pcOutputString, "%s %s\r\n", incorrect_passwd_msg, user_name);
							hal_cli_data_tx((signed char *)pcOutputString, strlen(pcOutputString));
						}
						hal_cli_data_tx((signed char *)input_passwd_msg, strlen(input_passwd_msg));
					}
					else
					{
						permission = PASSWD_CORRECT;
						goto exit_correct;
					}
					goto exit_incorrect;

				}

				/* Pass the received command to the command interpreter.  The
				command interpreter is called repeatedly until it returns
				pdFALSE	(indicating there is no more output) as it might
				generate more than one string. */
				do
				{
					/* Get the next output string from the command interpreter. */
					xReturned = mid_cli_parse_command(pcOutputString, in_string, CLI_STRING_MAX_OUTPUT_SIZE);

					/* Write the generated string to the UART. */
					hal_cli_data_tx((signed char *)pcOutputString, strlen(pcOutputString));
				} while(xReturned != pdFALSE);
				exit_correct:
				sprintf((signed char *)pcOutputString, "<%s> ", user_name);
				hal_cli_data_tx((signed char *)pcOutputString, strlen(pcOutputString));
				exit_incorrect:
				/* All the strings generated by the input command have been
				sent.  Clear the input string ready to receive the next command.
				Remember the command that was just processed first in case it is
				to be processed again. */
				in_index = 0;
				memset(in_string, 0x00, cmdMAX_INPUT_SIZE);
			}
			else
			{
				if(rx_char == cmdASCII_HEADLINE)
				{
					/* Ignore the character. */
				}
				else if((rx_char == cmdASCII_BS) || (rx_char == cmdASCII_DEL))		/* Backspace || Delete */
				{
					/* Backspace was pressed.  Erase the last character in the
					string - if any. */
					if(in_index > 0)
					{
						in_index--;
						in_string[in_index] = cmdASCII_STRINGEND;
						hal_cli_data_tx((signed char *)backspace, strlen(backspace));
					}
				}
				else
				{
					/* A character was entered.  Add it to the string entered so
					far.  When a \n is entered the complete	string will be
					passed to the command interpreter. */
					if((rx_char >= cmdASCII_SPACE) && (rx_char <= cmdASCII_TILDE))
					{
						if(in_index < cmdMAX_INPUT_SIZE)
						{
							in_string[in_index] = rx_char;
							in_index++;
						}
					}
				}
			}
			
			/* Must ensure to give the mutex back. */
			xSemaphoreGive(xTxMutex);
		}
	}
}

BaseType_t mid_cli_parse_command(char *dest, const char * const input, size_t len)
{
	static const struct cli_module_list_t *cmd = NULL;
	BaseType_t xReturn = pdTRUE;
	const char *pcRegisteredCommandString;
	size_t xCommandStringLength;
	const static char log1[] = "not recognised. Enter 'help' to view available commands.\r\n";

	/* Note:  This function is not re-entrant.  It must not be called from more
	thank one task. */

	if(cmd == NULL)
	{
		/* Search for the command string in the list of registered commands. */
		for(cmd = &cmd_list_head; cmd != NULL; cmd = cmd->next)
		{
			pcRegisteredCommandString = cmd->module->command;
			xCommandStringLength = strlen(pcRegisteredCommandString);

			/* To ensure the string lengths match exactly, so as not to pick up
			a sub-string of a longer command, check the byte after the expected
			end of the string is either the end of the string or a space before
			a parameter. */
			if((input[xCommandStringLength] == ' ') || (input[xCommandStringLength] == 0))
			{
				if(strncmp(input, pcRegisteredCommandString, xCommandStringLength) == 0)
				{
					/* The command has been found.  Check it has the expected
					number of parameters.  If expect_parame_num is -1,
					then there could be a variable number of parameters and no
					check is made. */
					if(get_num_of_parame(input) != cmd->module->expect_parame_num)
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
		/* The input command was found, but the number of parameters with the command
		was incorrect. */
		sprintf(dest, "\t'%s' %s", input, log1);
		cmd = NULL;
	}
	else if(cmd != NULL)
	{
		memset(dest, cmdASCII_STRINGEND, sizeof(char) * CLI_STRING_MAX_OUTPUT_SIZE);
		/* Call the callback function that is registered to this command. */
		xReturn = cmd->module->handle(dest, input, cmd->module->help_info);

		/* If xReturn is pdFALSE, then no further strings will be returned
		after this one, and	cmd can be reset to NULL ready to search
		for the next entered command. */
		if(xReturn == pdFALSE)
		{
			cmd = NULL;
		}
	}
	else
	{
		/* cmd was NULL, the command was not found. */
        if(*input != 0)
            sprintf(dest, "\t'%s' %s", input, log1);
		else
			memset(dest, cmdASCII_STRINGEND, sizeof(char) * CLI_STRING_MAX_OUTPUT_SIZE);
		xReturn = pdFALSE;
	}
	
	return xReturn;
}
/*-----------------------------------------------------------*/

__inline char *mid_cli_output_buffer(void)
{
	return out_buffer;
}

/*-----------------------------------------------------------*/

const char *FreeRTOS_CLIGetParameter(const char *commandString, UBaseType_t uxWantedParameter, BaseType_t *pxParameterStringLength)
{
	UBaseType_t uxParametersFound = 0;
	const char *pcReturn = NULL;

	*pxParameterStringLength = 0;

	while(uxParametersFound < uxWantedParameter)
	{
		/* Index the character pointer past the current word.  If this is the start
		of the command string then the first word is the command itself. */
		while((( *commandString ) != 0x00) && ((*commandString ) != ' '))
		{
			commandString++;
		}

		/* Find the start of the next string. */
		while(((*commandString) != 0x00) && ((*commandString) == ' '))
		{
			commandString++;
		}

		/* Was a string found? */
		if(*commandString != 0x00)
		{
			/* Is this the start of the required parameter? */
			uxParametersFound++;

			if(uxParametersFound == uxWantedParameter)
			{
				/* How long is the parameter? */
				pcReturn = commandString;
				while(((*commandString) != 0x00) && ((*commandString) != ' '))
				{
					(*pxParameterStringLength)++;
					commandString++;
				}

				if(*pxParameterStringLength == 0)
				{
					pcReturn = NULL;
				}

				break;
			}
		}
		else
		{
			break;
		}
	}

	return pcReturn;
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

static int8_t get_num_of_parame(const char *commandString)
{
	int8_t cParameters = 0;
	BaseType_t xLastCharacterWasSpace = pdFALSE;

	/* Count the number of space delimited words in commandString. */
	while(*commandString != 0x00)
	{
		if((*commandString) == ' ')
		{
			if(xLastCharacterWasSpace != pdTRUE)
			{
				cParameters++;
				xLastCharacterWasSpace = pdTRUE;
			}
		}
		else
		{
			xLastCharacterWasSpace = pdFALSE;
		}

		commandString++;
	}

	/* If the command string ended with spaces, then there will have been too
	many parameters counted. */
	if(xLastCharacterWasSpace == pdTRUE)
	{
		cParameters--;
	}

	/* The value returned is one less than the number of space delimited words,
	as the first word should be the command itself. */
	return cParameters;
}

static BaseType_t help_main( char *dest, const char * const src, const char * const help_info)
{
	struct cli_module_list_t *cmd_index = NULL;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) help_info;
	( void ) src;
	configASSERT( dest );

	/* Generate a table of task stats. */
	sprintf(dest, "%s", help_info);
	//cmd_index = mid_cli_cmd_list_head();
	for(cmd_index = mid_cli_cmd_list_head(); cmd_index != NULL; cmd_index = cmd_index->next)
    {
        strcat(dest,  "\t");
        strcat(dest, cmd_index->module->command);
        strcat(dest, ": ");
        strcat(dest, cmd_index->module->help_info);
    }
	
	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}


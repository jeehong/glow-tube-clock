/**  @file Mid_cli.h
 * @brief 串口命令行功能
 * @details 该功能运用调试串口，在终端上实现人机交互，
 			通过输入命令得到返回结果，以实现对MCU控制或状态监测。
 *
 * Example usage:\n
	1)创建一个命令结构体，设定相关信息
		build_var(cmd, help_info, parame_num);
			cmd: 命令名称；
			help_info: 帮助信息，输入各参数所代表的含义；
			parame_num: 期望输入的参数个数，输入参数不匹配时，命令行无法识别；参数不包括命令本身；
	2)创建一个函数，这个函数用于命令参数解析和功能执行；
		static BaseType_t cmd##_handle( char *dest, argv_attribute argv, const char * const help_info);
			字符串cmd必须与build_var中的cmd保持一致；
			dest: 需要返回信息时，向该地址写入字符串数据，如果需要多项数据返回，可参考info_handle函数结构；
			argv: 提供给命令执行函数各个参数信息
				例如命令cp -r src dest，则argv[0] = "cp",argv[1] = "-r",argv[2] = "src",argv[3] = "dest"
			help_info: 提供命令在构建时的帮助信息所在地址；
	3)将命令注册到命令行功能中:
		mid_cli_register(&cmd);
			cmd: 构建命令时的命令名称；
			将上面的函数放到app_cli_register( void )函数中调用；
	\n
日期       | 修改人    |描述
---------- | --------- | -------------
2017/04/25 | 姬宏光    |创建

*/

/** @defgroup Mid_cli 串口命令行功能
*/
#ifndef __MID_CLI_H__
#define __MID_CLI_H__

#include "common_type.h"
/* FreeRTOS includes. */
#include "os_inc.h"

#ifndef CLI_SUPPORT_PASSWD
	#define CLI_SUPPORT_PASSWD
#endif

/** @breif 定义每个参数最大字符长度为19个+'\0'*/
#define	cmdMAX_STRING_SIZE		20
/** @breif 定义每个输入命令的字符串最大参数个数为7个，包含命令本身*/
#define	cmdMAX_VARS_SIZE		10

/** @breif 命令传入时参数属性类型*/
typedef char** const argv_attribute;

/** @breif 命令触发回调函数结构类型*/
typedef BaseType_t (*module_func_handle)(char * dest, argv_attribute argv, const char * const help_info);

/** @breif 命令参数结构体*/
typedef struct _command_t
{
	const char * const command;			/**< 命令*/
	const char * const help_info;		/**< 帮助信息*/
	const module_func_handle handle;	/**< 执行函数*/
	U8 expect_parame_num;	/**< 期望输入的参数个数，不包含命令本身*/
} command_t;

/** @ingroup Mid_cli
*
* 为新的命令创建一个回调
* 
* @param dest 目的内存地址，将输出信息传入该地址
* 
* @param argv 终端输入的命令信息，每个空格切断的字符串放入每一个结构体数组
*
* @param help_info 该命令所对应的帮助信息首地址
*
* @return 
*	pdTRUE: 退出该函数后，会继续在此进入，该回调函数
*	pdFALSE: 退出后结束该回调调用
*/
#define cmd_handle(cmd) static BaseType_t cmd##_main(char *dest, argv_attribute argv, const char * const help_info)

/** @ingroup Mid_cli
*
* 为新的命令创建一个数据结构
* 
* @param var 命令前缀，命令最基本的参数
* 
* @param help 帮助信息，字符串类型
*
* @param want 期望的参数个数，不包含命令前缀本身
*
* @return 无返回值
*/
#define	build_var(var, help, want) 			\
cmd_handle(var);		\
static const struct _command_t var =	{	#var, (help), var##_main, (want)	}

#define cli_malloc(size)	pvPortMalloc(size)

/** @ingroup Mid_cli
*
* 注册一个新的命令
* 
* @param p 新命令的数据指针
*
* @return 
返回值|描述
------- | --------- 
pdFAIL | 注册失败
pdPASS  | 注册成功
*/
BaseType_t mid_cli_register(const struct _command_t *const p);

/** @ingroup Mid_cli
*
* 初始化命令行功能
* 默认系统仅提供一个帮助命令 help
* help 命令会将当前可用的命令以及相应的帮助信息打印出来
* 
* @param usStackSize 命令行功能分配的栈尺寸
*
* @param uxPriority 命令行功能任务优先级
*
* @param prjPrefix 命令返回值，返回前缀，一般设置为项目代号+版本号: "Whale-1.0.0 "
*
* @return 无返回值
*/
void mid_cli_init(U16 usStackSize, UBaseType_t uxPriority, char *t, TaskHandle_t *h);


#endif /* COMMAND_INTERPRETER_H */



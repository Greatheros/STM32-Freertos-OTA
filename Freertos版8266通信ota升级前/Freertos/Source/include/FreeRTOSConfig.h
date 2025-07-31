#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H


#define configUSE_PREEMPTION		1
#define configUSE_TICK_HOOK			0
#define configCPU_CLOCK_HZ			( ( unsigned long ) 72000000 )	
#define configTICK_RATE_HZ			( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES		( 10 )
#define configMINIMAL_STACK_SIZE	( ( unsigned short ) 64 )
#define configTOTAL_HEAP_SIZE		( ( size_t ) ( 9050 ) )    //9*1024
#define configMAX_TASK_NAME_LEN		( 16 )
#define configUSE_TRACE_FACILITY	0
#define configUSE_16_BIT_TICKS		0
#define configIDLE_SHOULD_YIELD		1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete				1
#define INCLUDE_vTaskCleanUpResources	0
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1

/* This is the raw value as per the Cortex-M3 NVIC.  Values can be 255
(lowest) to 0 (1?) (highest). */
#define configKERNEL_INTERRUPT_PRIORITY 		255
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	191 /* equivalent to 0xb0, or priority 11. */


/* This is the value being used as per the ST library which permits 16
priority values, 0 to 15.  This must correspond to the
configKERNEL_INTERRUPT_PRIORITY setting.  Here 15 corresponds to the lowest
NVIC value of 255. */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY	15

//获取堆栈剩余
#define INCLUDE_uxTaskGetStackHighWaterMark             1
//时间片调度
#define configUSE_TIME_SLICING                          1
//抢占式调度
#define configUSE_PREEMPTION                            1
//使能低功耗 Tickless 模式
#define configUSE_TICKLESS_IDLE                         1
//系统进入相应低功耗模式的最短时长，2
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP           2
//使用互斥锁
#define configUSE_MUTEXES                               1
//启用静态分配
#define configSUPPORT_STATIC_ALLOCATION   1
/* 使能软件定时器*/
#define configUSE_TIMERS 1         
 /* 定义软件定时器任务的优先级 */                                 
#define configTIMER_TASK_PRIORITY (3)       
 /* 定义软件定时器命令队列的长度*/
#define configTIMER_QUEUE_LENGTH           2
/* 定义软件定时器任务的栈空间大小*/
#define configTIMER_TASK_STACK_DEPTH (180) 
/*启动空闲任务钩子函数*/
#define configUSE_IDLE_HOOK    1
//获取任务句柄
#define INCLUDE_xTaskGetHandle 1
//获取空闲任务句柄
#define INCLUDE_xTaskGetIdleTaskHandle 1

#define configCHECK_FOR_STACK_OVERFLOW 2

//必须添加的宏
#define configSUPPORT_DYNAMIC_ALLOCATION 1

#define xPortPendSVHandler  PendSV_Handler
#define vPortSVCHandler     SVC_Handler
#define INCLUDE_xTaskGetSchedulerState   1




#endif /* FREERTOS_CONFIG_H */


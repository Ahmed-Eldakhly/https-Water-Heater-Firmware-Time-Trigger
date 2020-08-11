/*******************************************************************************
 *  
 * File:                scheduler.h
 * 
 * Author:              Ahmed Eldakhly
 * 
 * Comments:            it contains Scheduler prototypes functions(service layer).
 * 
 * Revision history:    3/3/2020
 * 
 ******************************************************************************/

/* This is a guard condition so that contents of this file are not included
 * more than once. */
#ifndef Scheduler_H_
#define Scheduler_H_

/*******************************************************************************
 *                              Included Libraries                             *
 *******************************************************************************/
#include "scheduler_config.h"
#include "stdtypes.h"

/*******************************************************************************
 *                              Types Declaration                              *
 *******************************************************************************/

/****************************Typedef for Tick.time******************************/
typedef char TickTime;

/************************EnumScheduler_RepeatedFunction_t***********************/
typedef enum {
	ONCE,
	REPEAT
}EnumScheduler_RepeatedFunction_t;

/****************************EnumScheduler_Status_t*****************************/
typedef enum {
	SCHEDULER_NOT_AVAILABLE_TICK_TIME_ERROR,
    SCHEDULER_TASKS_QUEUE_IS_FULL_ERROR,
    SCHEDULER_ID_IS_NOT_EXIST_ERROR,        
    SCHEDULER_NO_ERROR
}EnumScheduler_Error_t;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

/*******************************************************************************
 * Function Name:	Scheduler_Initialization
 *
 * Description: 	Initialize the Scheduler Driver
 *
 * Inputs:			TickTime                (typedef for u8 with specific values)
 *
 * Outputs:			NULL
 *
 * Return:			EnumScheduler_Error_t   (typedef for scheduler errors). 
 *******************************************************************************/
extern EnumScheduler_Error_t Scheduler_Initialization(TickTime a_u8tickTme);

/*******************************************************************************
 * Function Name:	Scheduler_StartFunction
 *
 * Description: 	Add Function to Scheduler Queue
 *
 * Inputs:			Pointer to function
 * 					uint8   (Function's ID)
 * 					uint16  (Period of function)
 *                  uint16  (start time for this task)
 * 					EnumScheduler_RepeatedFunction_t (Repeat Type)
 *
 * Outputs:			NULL
 *
 * Return:			EnumScheduler_Error_t   (typedef for scheduler errors).
 *******************************************************************************/
extern EnumScheduler_Error_t Scheduler_StartFunction(void (*task_function_ptr) (void),
        uint8 a_u8task_ID , uint16 a_u16task_periodicity , uint16 a_u16task_start_time ,
                                            EnumScheduler_RepeatedFunction_t a_RepeatType_t);

/*******************************************************************************
 * Function Name:	Scheduler_StopFunction
 *
 * Description: 	Remove Function from Scheduler Queue
 *
 * Inputs:			uint8 (Function's ID)
 *
 * Outputs:			NULL
 *
 * Return:			EnumScheduler_Error_t   (typedef for scheduler errors).
 *******************************************************************************/
extern EnumScheduler_Error_t Scheduler_StopFunction(uint8 a_u8_task_ID);

/*******************************************************************************
 * Function Name:	Scheduler_Dispatcher
 *
 * Description: 	Put Ready Functions on Running state
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
extern void Scheduler_Dispatcher(void);


#endif /* Scheduler_H_ */
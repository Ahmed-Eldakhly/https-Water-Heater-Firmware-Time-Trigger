/*******************************************************************************
 *  
 * File:                scheduler.c
 * 
 * Author:              Ahmed Eldakhly
 * 
 * Comments:            it contains Scheduler functions (service layer).
 * 
 * Revision history:    9/7/2020
 * 
 ******************************************************************************/

/*******************************************************************************
 *                       	Included Libraries                                 *
 *******************************************************************************/
#include "scheduler.h"
#include "timers.h"
#include "interrupt.h"
#include "CPU_sleep.h"
/*******************************************************************************
 *                         		Types Declaration                              *
 *******************************************************************************/

/********************************EnumScheduler_Status_t*******************************/
typedef enum {
	READY,
	RUNNING,
	WAITING
}EnumScheduler_Status_t;

/*******************************StructScheduler_Object_t******************************/
typedef struct {
	uint8                               Task_ID;
	uint16                              Task_ExecutionTime;
	uint16                              Task_Periodicity;
	EnumScheduler_RepeatedFunction_t    Task_RepeatType;
	EnumScheduler_Status_t              Task_State;
	void                                (*Task_Function)(void);
}StructScheduler_Object_t;

/*******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************/
/* minimum time to call dispatcher */
volatile static uint16 g_u16_TickTimeCounter = 0;
/* queue of tasks */
static StructScheduler_Object_t Scheduler_Tasks[MAX_TASKS];
/* value that the timer need to get the specified tick time */
uint32 u32compare_value;

/*******************************************************************************
 *                                 Definitions                                 *
 *******************************************************************************/
#define COUNTER32BITS				4294967295u
#define COUNTER16BITS				65535u
/*******************************************************************************
 *                      Static Functions Definitions                           *
 *******************************************************************************/

/*******************************************************************************
 * Function Name:	Scheduler_Core
 *
 * Description: 	Call Back Function for Timer ISR
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			Status to check function execution
 *******************************************************************************/
static void Scheduler_Core(void)
{
	/* increase User Tick.Time */
	g_u16_TickTimeCounter++;
    /* value that the timer counter start with to get the specified tick time */
    Timer1_write_counter(COUNTER16BITS - u32compare_value);
	/* for loop to check if any function in Scheduler Queue get it time to execute */
	for(uint16 Counter = 0 ; Counter < MAX_TASKS ; Counter++)
	{
		/* check if any function in Scheduler Queue get it time to execute */
		if((Scheduler_Tasks[Counter].Task_ExecutionTime == g_u16_TickTimeCounter)
				&& Scheduler_Tasks[Counter].Task_ID != 0 )
		{
			/* convert function to ready state */
			Scheduler_Tasks[Counter].Task_State = READY;
			/* set the next time to execute this task again */
            Scheduler_Tasks[Counter].Task_ExecutionTime += Scheduler_Tasks[Counter].Task_Periodicity;
		}
		else
		{
			/*Do Nothing*/
		}
	}
}

/*******************************************************************************
 * Function Name:	Scheduler_Tick_time_calculation
 *
 * Description: 	select suitable initialization of timer to get the specified
 *                  tick time.
 *
 * Inputs:			TickTime                (typedef for u8 with specific values)
 *
 * Outputs:			NULL
 *
 * Return:			EnumScheduler_Error_t   (typedef for scheduler errors). 
 *******************************************************************************/
static EnumScheduler_Error_t Scheduler_Tick_time_calculation(TickTime a_u8tickTme)
{
    /* variable to know if error occur */
    EnumScheduler_Error_t tick_calculation_t = SCHEDULER_NO_ERROR;
    /* tick time calculation */
    u32compare_value = (FREQUENCY / 4000) * a_u8tickTme;
    /* check if calculation suitable with hardware timer specifications */
    if(u32compare_value < COUNTER16BITS)
    {
        Timer1_config_t.timer_prescaler_t = TIMER1_NO_PRESCALER;
        /*Initialize Timer 0*/
        Timer1_Initialization();
        tick_calculation_t = SCHEDULER_NO_ERROR;
    }
    else if (u32compare_value / 2 < COUNTER16BITS)
    {
        Timer1_config_t.timer_prescaler_t = TIMER1_PRESCALER_2;
        u32compare_value /= 2;
        /*Initialize Timer 0*/
        Timer1_Initialization();
        tick_calculation_t = SCHEDULER_NO_ERROR;
    }
    else if (u32compare_value / 4 < COUNTER16BITS)
    {
        Timer1_config_t.timer_prescaler_t = TIMER1_PRESCALER_4;
        u32compare_value /= 4;
        /*Initialize Timer 0*/
        Timer1_Initialization();
        tick_calculation_t = SCHEDULER_NO_ERROR;        
    }
    else if (u32compare_value / 8 < COUNTER16BITS)
    {
        Timer1_config_t.timer_prescaler_t = TIMER1_PRESCALER_8;
        u32compare_value /= 8;
        /*Initialize Timer 0*/
        Timer1_Initialization(); 
        tick_calculation_t = SCHEDULER_NO_ERROR;
    }
    else
    {
        tick_calculation_t = SCHEDULER_NOT_AVAILABLE_TICK_TIME_ERROR;
    }
    return tick_calculation_t;
}
/*******************************************************************************
 *                      extern Functions Definitions                           *
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
EnumScheduler_Error_t Scheduler_Initialization(TickTime a_u8tickTme)
{
    /* variable to know if error occur */
    EnumScheduler_Error_t initialization_error_t = SCHEDULER_NO_ERROR;
    /* calculate correct start value of timer to get the specified tick time */
    initialization_error_t = Scheduler_Tick_time_calculation(a_u8tickTme);
    /* check if the specified tick time has been set correctly */
    if(initialization_error_t == SCHEDULER_NO_ERROR)
    {
        /* Send Call Back function to Timer ISR */
        Timer1_set_callback_function(Scheduler_Core);
        /* Enable Global interrupt */
        Global_interrupt_enable();
        /* Enable Peripheral interrupts. */
        Peripherals_interrupt_enable();
        /*for loop to Initialize all members in Scheduler Queue*/
        for(uint16 Counter = 0 ; Counter < MAX_TASKS ; Counter++)
        {
            Scheduler_Tasks[Counter].Task_ID            = 0;
            Scheduler_Tasks[Counter].Task_Periodicity   = 0;
            Scheduler_Tasks[Counter].Task_RepeatType    = ONCE;
            Scheduler_Tasks[Counter].Task_ExecutionTime = 0;
            Scheduler_Tasks[Counter].Task_State         = WAITING;
            Scheduler_Tasks[Counter].Task_Function      = NULL_PTR;
        }
        /* start the scheduler */
        Timer1_enable();
    }
    else
    {
        initialization_error_t = SCHEDULER_NOT_AVAILABLE_TICK_TIME_ERROR;
    }
    return initialization_error_t;
}

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
EnumScheduler_Error_t Scheduler_StartFunction(void (*task_function_ptr) (void), uint8 a_u8_task_ID ,
		uint16 a_u16_task_periodicity , uint16 a_u16_task_start_time ,
        EnumScheduler_RepeatedFunction_t Enum_Task_RepeatType)
{
    /* variable to know if error occur */
    EnumScheduler_Error_t starting_task_error = SCHEDULER_NO_ERROR;
	/*Enable Timer0 overflow interrupt */
	Timer1_enable_overflow_interrupt();
	/*Put User Function In Scheduler Queue*/
	for(uint16 Counter = 0 ; Counter < MAX_TASKS ; Counter++)
	{
		if(Scheduler_Tasks[Counter].Task_ID == 0)
		{
			Scheduler_Tasks[Counter].Task_ID            = a_u8_task_ID;
			Scheduler_Tasks[Counter].Task_Periodicity   = a_u16_task_periodicity;
			Scheduler_Tasks[Counter].Task_RepeatType    = Enum_Task_RepeatType;
			Scheduler_Tasks[Counter].Task_ExecutionTime = a_u16_task_start_time + g_u16_TickTimeCounter + 1;
			Scheduler_Tasks[Counter].Task_State         = WAITING;
			Scheduler_Tasks[Counter].Task_Function      = task_function_ptr;
            starting_task_error = SCHEDULER_NO_ERROR;
			break;
		}
		else
		{
			starting_task_error = SCHEDULER_TASKS_QUEUE_IS_FULL_ERROR;
		}
	}
    return starting_task_error;
}

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
EnumScheduler_Error_t Scheduler_StopFunction(uint8 a_u8_task_ID)
{
    /* variable to know if error occur */
    EnumScheduler_Error_t stoping_task_error = SCHEDULER_NO_ERROR;
	/*Remove Selected Function From Scheduler Queue*/
	for(uint16 Counter=0;Counter<MAX_TASKS;Counter++)
	{
		if(Scheduler_Tasks[Counter].Task_ID == a_u8_task_ID)
		{
			Scheduler_Tasks[Counter].Task_ID            = 0;
			Scheduler_Tasks[Counter].Task_Periodicity   = 0;
			Scheduler_Tasks[Counter].Task_RepeatType    = ONCE;
			Scheduler_Tasks[Counter].Task_ExecutionTime = 0;
			Scheduler_Tasks[Counter].Task_State         = WAITING;
			Scheduler_Tasks[Counter].Task_Function      = NULL_PTR;
            stoping_task_error = SCHEDULER_NO_ERROR;
			break;
		}
		else
		{
			stoping_task_error = SCHEDULER_ID_IS_NOT_EXIST_ERROR;
		}
	}
	/*Check Variable To Disable Timer Interrupt if it isn't any functions in Scheduler Queue*/
	uint16 Counter = 0 ;
	/*for loop to check if it isn't any functions in Scheduler Queue*/
	for(; Counter < MAX_TASKS ; Counter++)
	{
		if(Scheduler_Tasks[Counter].Task_ID != 0)
		{
			break;
		}
		else
		{
			/*Do Nothing*/
		}
	}
	/*check if it isn't any functions in Scheduler Queue*/
	if(Counter == MAX_TASKS)
	{
		/*close Timer interrupt because Scheduler Queue is Empty*/
		Timer1_disable_overflow_interrupt();
	}
	else
	{
		/*Do Nothing*/
	}
    return stoping_task_error;
}

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
void Scheduler_Dispatcher(void)
{
	/*for loop to execute ready functions in Scheduler queue*/
	for(uint16 Counter = 0 ; Counter < MAX_TASKS ; Counter++)
	{
		/*check if function is ready to execute*/
		if(Scheduler_Tasks[Counter].Task_State == READY)
		{
			Scheduler_Tasks[Counter].Task_State = RUNNING;
			Scheduler_Tasks[Counter].Task_Function();
			Scheduler_Tasks[Counter].Task_State = WAITING; 
            /*check if function should execute one time and thin remove from Scheduler Queue*/
            if(Scheduler_Tasks[Counter].Task_RepeatType == ONCE)
            {
                /*Remove once time function from Scheduler Queue*/
                Scheduler_Tasks[Counter].Task_ID            = 0;
                Scheduler_Tasks[Counter].Task_Periodicity   = 0;
                Scheduler_Tasks[Counter].Task_RepeatType    = ONCE;
                Scheduler_Tasks[Counter].Task_ExecutionTime = 0;
                Scheduler_Tasks[Counter].Task_State         = WAITING;
                Scheduler_Tasks[Counter].Task_Function      = NULL_PTR;
            }
            else
            {
                /*Do Nothing*/
            }
		}
		else
		{
			/*Do Nothing*/
		}
	}
}


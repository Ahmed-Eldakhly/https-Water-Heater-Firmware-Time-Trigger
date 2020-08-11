/*******************************************************************************
 *  
 * File:                main.c 
 * 
 * Author:              Ahmed Eldakhly
 * 
 * Comments:            the application code.
 * 
 * Revision history:    13/7/2020
 * 
 ******************************************************************************/

/*******************************************************************************
 *                       	  Included Libraries                               *
 *******************************************************************************/
#include "heater.h"
#include "scheduler.h"

/*******************************************************************************
 *                            Prototype Function                               *
 *******************************************************************************/
void initialization_Tasks(void);

/* main application code */
void main(void)
{
    /* Initialize all peripherals that will be used in this device. */
    Heater_Initialization();
    /* Set the time tick of the scheduler 1ms. */
    Scheduler_Initialization(TICK_1_MS_);
    /* this task start immediately for one time to initialize all tasks to put them in scheduler queue. */
    Scheduler_StartFunction(initialization_Tasks , 1 , 1 , 0 , ONCE);
    /* loop to make program is worked as long the power is on */
    while(1)
    {
        /* run the scheduler dispatcher to run tasks at their time */
        Scheduler_Dispatcher();
    }
}

/*******************************************************************************
 * Function Name:	initialization_Tasks
 *
 * Description: 	Add all tasks to the Scheduler queue.
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void initialization_Tasks(void)
{
    /* this task start immediately to get the on - off push button state and will be repeated every 10 ms. */
    Scheduler_StartFunction(Heater_On_Off_Button_Task , 1 , 10 , 0 , REPEAT);
    /* this task start after 1 ms to get the Up -Down push buttons state and will be repeated every 10 ms. */
    Scheduler_StartFunction(Heater_Up_Down_Buttons_Task , 2 , 10 , 1 , REPEAT);
    /* this task start after 3 ms to display the temperature on 7 segments and will be repeated every 10 ms. */
    Scheduler_StartFunction(Heater_Display_Task , 3 , 10 , 3 , REPEAT);
    /* this task start after 5 ms to get the temperature sensor reading and will be repeated every 100 ms. */
    Scheduler_StartFunction(Heater_Reading_Temperature_Sensor_Task , 4 , 100 , 5 , REPEAT);
    /* this task start after 6 ms to control the heater and the cooler and will be repeated every 100 ms. */
    Scheduler_StartFunction(Heater_Heating_Cooling_Task , 5 , 100 , 6 , REPEAT);
    /* this task start after 7 ms to control the heating LED and will be repeated every 100 ms. */
    Scheduler_StartFunction(Heater_LED_Task , 6 , 100 , 7 , REPEAT);
}

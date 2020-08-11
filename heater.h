/*******************************************************************************
 *  
 * File:                heater.h 
 * 
 * Author:              Ahmed Eldakhly
 * 
 * Comments:            it contains prototypes, enums and definitions of
 *                      heater Module.
 * 
 * Revision history:    13/7/2020
 * 
 ******************************************************************************/

/* This is a guard condition so that contents of this file are not included
   more than once. */
#ifndef HEATER_H
#define	HEATER_H

/*******************************************************************************
 *                       	Included Libraries                                 *
 *******************************************************************************/
#include "GPIO.h"
#include "stdtypes.h"
#include "CPU_sleep.h"
#include "heater_config.h"

/*******************************************************************************
 *                              Definitions                                    *
 *******************************************************************************/

/*******************************************************************************
 *                             Functions Prototypes                            *
 *******************************************************************************/

/*******************************************************************************
 * Function Name:	Heater_Initialization
 *
 * Description: 	Initialize all peripherals will be used in the module.
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void Heater_Initialization(void);

/*******************************************************************************
 * Function Name:	Heater_Display_Task
 *
 * Description: 	Display on Seven Segment Task at any state of device states.
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void Heater_Display_Task(void);

/*******************************************************************************
 * Function Name:	Heater_On_Off_Button_Task
 *
 * Description: 	Get On/Off push button states Task to turn the heater on or off.
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void Heater_On_Off_Button_Task(void);

/*******************************************************************************
 * Function Name:	Heater_Up_Down_Buttons_Task
 *
 * Description: 	Get up and down push buttons states Task at any state of 
 *                  device states.
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void Heater_Up_Down_Buttons_Task(void);

/*******************************************************************************
 * Function Name:	Heater_LED_Task
 *
 * Description: 	Turn the LED on/off or blank it based on device state and 
 *                  heater or cooler state.
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void Heater_LED_Task(void);

/*******************************************************************************
 * Function Name:	Heater_Heating_Cooling_Task
 *
 * Description: 	turn on the cooler or the heater based on the average of
 *                  the last ten readings values of the temperature sensor.
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void Heater_Heating_Cooling_Task(void);

/*******************************************************************************
 * Function Name:	Heater_Reading_Temperature_Sensor_Task
 *
 * Description: 	get new reading from temperature sensor every 100ms and add
 *                  it to the array for last 10 readings then get the average.
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void Heater_Reading_Temperature_Sensor_Task(void);

#endif	/* HEATER_H */


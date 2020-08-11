/*******************************************************************************
 *  
 * File:                scheduler_config.h
 * 
 * Author:              Ahmed Eldakhly
 * 
 * Comments:            it contains Scheduler configuration (service layer).
 * 
 * Revision history:    3/3/2020
 * 
 ******************************************************************************/

/* This is a guard condition so that contents of this file are not included
 * more than once. */
#ifndef SCHEDULER_CONFIG_H_
#define SCHEDULER_CONFIG_H_

/*************defines for allowable Tick.time values**************/
/*you can add any new define for your  Tech.time with the number represent it in ms*/
#define TICK_1_MS_ 		1u
#define TICK_2_MS_		2u
#define TICK_5_MS_ 		5u
#define TICK_10_MS_ 	10u

/**************define for Maximum functions in Scheduler****************/
#define MAX_TASKS       10u

/********************Frequency of micro_controller**********************/
#define FREQUENCY       8000000u

#endif /* SCHEDULER_CONFIG_H_ */



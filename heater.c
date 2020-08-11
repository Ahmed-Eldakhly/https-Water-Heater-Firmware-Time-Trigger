/*********************************************************************************
 *  
 * File:                heater.c 
 * 
 * Author:              Ahmed Eldakhly
 * 
 * Comments:            it contains functions implementation of heater Module.
 * 
 * Revision history:    13/7/2020
 * 
 ********************************************************************************/

/*******************************************************************************
 *                       	Included Libraries                                 *
 *******************************************************************************/
#include "general_bitConfig.h"
#include "ADC.h"
#include "timers.h"
#include "SevenSegment.h"
#include "SevenSegment_unportablePart.h"
#include "EEPROM24c04.h"
#include "interrupt.h"
#include "heater.h"
#include "heater_config.h"

/*******************************************************************************
 *                             Types Definition                                *
 *******************************************************************************/
#define ENABLE                          HIGH
#define DISABLE                         LOW

/*******************************************************************************
 *                             Types Declaration                               *
 *******************************************************************************/

/***************************** EnumHeater_State_t ******************************/
typedef enum{
    HEATER_OFF_STATE,
    HEATER_ON_STATE,
    HEATER_SET_TEMERATURE
}EnumHeater_State_t;

/***************************** EnumButton_State_t ******************************/
typedef enum{
    PUSH_BUTTON_IS_PRESSED,
    PUSH_BUTTON_IS_NOT_RELEASED,        
    PUSH_BUTTON_DEBOUNCING,
    PUSH_BUTTON_IS_NOT_PRESSED
}EnumButton_State_t;

/*******************************************************************************
 *                           Static Variables                                  *
 *******************************************************************************/
/* System state variable that used to make system state machine. */
static EnumHeater_State_t g_Heater_state_t = HEATER_OFF_STATE;
/* Selected temperature by user. */
static uint8 g_u8Heater_desired_temperature = 60;
/* real time temperature of water. */
static uint8 g_u8Heater_real_time_temperature = 60;
/* flag to synchronize between two Seven Segment to Display two different digits. */
static uint8 g_u8Sync_flag = 0;
/* flag to make sure the ADC read temperature sensor reading every 100ms. */
static uint8 g_u8calculate_temperature_flag = 0;
/* summation of last ten readings of temperature sensor to get the average. */
static uint16 g_u16Sum_of_ten_readings = 0;
/* counter to start get the average after 10 readings. */
static uint8 g_u8Reading_counter = 0;
/* the index of the reading that will be removed to add new reading from temperature sensor. */
static uint8 g_u8Reading_deleted_element = 0;
/* array of 10 elements to add last 10 readings from temperature sensor to calculate the average. */
static uint8 g_u8Reading_arr[10] = {0};
/* flag to blank the temperature every 1 second in the setup state. */
static uint8 g_u8Display_blank_counter = 0;
/* variable to get check if 5 seconds pass without pressing on any key in setup state to return to heating state. */
static uint8 g_u8End_setup_state = 0;
/* flag to blank the LED every 1 second in the heating process. */
static uint8 g_u8Led_blank_counter = 0;

/*******************************************************************************
 *                            Functions Definitions                            *
 *******************************************************************************/

/*******************************************************************************
 *                              Static Functions                               *
 *******************************************************************************/
/*******************************************************************************
 * Function Name:	Check_ON_Push_Button
 *
 * Description: 	Get the state of Push Button.
 *
 * Inputs:			uint8 a_u8Port:                 for push Button connecting port.
 *                  uint8 a_u8Pin:                  for push Button connecting pin.
 *                  pointer to EnumButton_State_t:  to know the last state of the button.
 *
 * Outputs:			NULL
 *
 * Return:			EnumButton_State_t: typedef for states of the Push Button.
 *******************************************************************************/
static EnumButton_State_t Check_ON_Push_Button(uint8 a_u8Port , uint8 a_u8Pin ,
                                               EnumButton_State_t * retVal_t)
{
    /* check on the button is pressed or not. */
    if(GPIO_ReadFromPin(a_u8Port , a_u8Pin) == PUSH_BUTTON_IS_PRESSED)
    {
        /* check on the last state of the button to update this state. */
        switch(*retVal_t)
        {
            case PUSH_BUTTON_IS_NOT_PRESSED:
                *retVal_t = PUSH_BUTTON_DEBOUNCING;
                break;
            case PUSH_BUTTON_DEBOUNCING:
                *retVal_t = PUSH_BUTTON_IS_NOT_RELEASED;
                break;
            case PUSH_BUTTON_IS_NOT_RELEASED:
                /* Do Nothing. */
                break;
            default:
                /* Do Nothing. */
                break;
        }
    }
    else 
    {
        /* check the state of the button before not pressed state. */
        if(*retVal_t == PUSH_BUTTON_IS_NOT_RELEASED)
        {
            /* The button was pressed and released. */
            *retVal_t = PUSH_BUTTON_IS_PRESSED;
        }
        else
        {
            /* The button was not pressed. */
            *retVal_t = PUSH_BUTTON_IS_NOT_PRESSED;
        }
    }
    /* return the status of button. */
    return *retVal_t;
}

/*******************************************************************************
 * Function Name:	Heater_turn_off
 *
 * Description: 	disable all peripherals in the device and reset all used 
 *                  variables and save last temperature on EEPROM for next use.
 *
 * Inputs:			NULL
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
static void Heater_turn_off(void)
{
    /* Disable all Seven Segments. */
    SevenSegment_SelectChip(SEVEN_SEGMENT_DISABLE_CHIP , SEVEN_SEGMENT_DISABLE_CHIP ,
                            SEVEN_SEGMENT_DISABLE_CHIP , SEVEN_SEGMENT_DISABLE_CHIP);
    /* Disable Cooler. */
    GPIO_WriteOnPin(COOLER_PORT , COOLER_PIN , DISABLE);
    /* Disable heating resistance. */
    GPIO_WriteOnPin(HEATING_RESISTANCE_PORT , HEATING_RESISTANCE_PIN , DISABLE);
    /* Disable heating LED. */
    GPIO_WriteOnPin(LEDS_PORT , LED5_PIN , DISABLE);
    /* reset all flags. */
    g_u8Sync_flag = 0;
    g_u16Sum_of_ten_readings = 0;
    g_u8Reading_counter = 0;
    g_u8Reading_deleted_element = 0;
    g_u8Display_blank_counter = 0;
    g_u8End_setup_state = 0;
    g_u8Led_blank_counter = 0;
    /* change the state of the device to Heating off state. */
    g_Heater_state_t = HEATER_OFF_STATE;
}

/*******************************************************************************
 *                              Extern Functions                               *
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
void Heater_Initialization(void)
{
    /* Initialize EEPROM to save last temperature before turn off the heater. */
    EEPROM_Initialization();
    /* Initialize Seven Segment to Display selected temperature. */
    SevenSegment_Initialization();
    /* Enable pull up resistor on portB. */
    GPIO_Enable_Pull_Up_On_PortB();
    /* Set Cooler pin as output to control the fan. */
    GPIO_SetPinDirection(COOLER_PORT , COOLER_PIN , OUTPUT);
    /* Enable ADC peripheral on channel 2 (A2) for temperature sensor. with pre_scaler 16 for 8MHZ clock. */
    ADC_Initialization(ADC_CHANNEL_2 , ADC_V_REF_DEFAULT_VALUE , ADC_V_REF_DEFAULT_VALUE);
    ADC_Select_prescaler(ADC_PRESCALER_16);
    /* Set Heating resistance pin as output to control the fan. */
    GPIO_SetPinDirection(HEATING_RESISTANCE_PORT , HEATING_RESISTANCE_PIN , OUTPUT);
    /* Set LED pin output to control the heating LED */
    GPIO_SetPinDirection(LEDS_PORT , LED1_PIN , OUTPUT);
    GPIO_SetPinDirection(LEDS_PORT , LED2_PIN , OUTPUT);
    GPIO_SetPinDirection(LEDS_PORT , LED3_PIN , OUTPUT);
    GPIO_SetPinDirection(LEDS_PORT , LED4_PIN , OUTPUT);
    GPIO_SetPinDirection(LEDS_PORT , LED5_PIN , OUTPUT);
    /* Set push button pin as input to get user input when he want to increase heater temperature. */
    GPIO_SetPinDirection(UP_PORT , UP_PIN , INPUT);
    /* Set push button pin as input to get user input when he want to decrease heater temperature. */
    GPIO_SetPinDirection(DOWN_PORT , DOWN_PIN , INPUT);
    /* Set push button pin as input to get user input when he want to turn on/off the device. */
    GPIO_SetPinDirection(ON_OFF_PORT , ON_OFF_PIN , INPUT);
    /* Select the edge for External interrupt that the interrupt will trigger. */
    External_interrupt_select_falling_edge();
    /* Enable external interrupt to wake the device up by it at the sleeping mood. */
    External_interrupt_enable();
}

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
void Heater_Display_Task(void)
{
    /* turn off all 7 segments. */
    SevenSegment_SelectChip(SEVEN_SEGMENT_DISABLE_CHIP , SEVEN_SEGMENT_DISABLE_CHIP ,
                            SEVEN_SEGMENT_DISABLE_CHIP , SEVEN_SEGMENT_DISABLE_CHIP);
    switch(g_Heater_state_t)
    {
        case HEATER_OFF_STATE:
            /* Do Nothing. */
            break;
        case HEATER_ON_STATE:
            /* if condition to change between two seven segment to display different numbers on them.*/
            if(g_u8Sync_flag == 0)
            {
                /* display the second digit of the temperature. */
                SevenSegment_Display(g_u8Heater_real_time_temperature / 10 ,
                                     SEVEN_SEGMENT_NOT_DISPLAY_DOT);
                /* Enable Seven segment number 2 only. */
                SevenSegment_SelectChip(SEVEN_SEGMENT_DISABLE_CHIP ,
                                        SEVEN_SEGMENT_ENABLE_CHIP ,
                                        SEVEN_SEGMENT_DISABLE_CHIP ,
                                        SEVEN_SEGMENT_DISABLE_CHIP);
                g_u8Sync_flag = 1;
            }
            else
            {
                /* display the first digit of the temperature. */
                SevenSegment_Display(g_u8Heater_real_time_temperature % 10 ,
                                     SEVEN_SEGMENT_NOT_DISPLAY_DOT);
                /* Enable Seven segment number 3 only. */
                SevenSegment_SelectChip(SEVEN_SEGMENT_DISABLE_CHIP ,
                                        SEVEN_SEGMENT_DISABLE_CHIP ,
                                        SEVEN_SEGMENT_ENABLE_CHIP ,
                                        SEVEN_SEGMENT_DISABLE_CHIP);
                g_u8Sync_flag = 0;
            }
            break;
        case HEATER_SET_TEMERATURE:
            /* check on blank flag to blank the temperature on the Seven Segments Every 1 second at setup state. */
            if(g_u8Display_blank_counter < 10)
            {
                /* if condition to change between two seven segment to display different numbers on them.*/
                if(g_u8Sync_flag == 0)
                {
                    /* display the second digit of the temperature. */
                    SevenSegment_Display(g_u8Heater_desired_temperature / 10 ,
                                         SEVEN_SEGMENT_NOT_DISPLAY_DOT);
                    /* Enable Seven segment number 2 only. */
                    SevenSegment_SelectChip(SEVEN_SEGMENT_DISABLE_CHIP ,
                                            SEVEN_SEGMENT_ENABLE_CHIP ,
                                            SEVEN_SEGMENT_DISABLE_CHIP ,
                                            SEVEN_SEGMENT_DISABLE_CHIP);
                    g_u8Sync_flag = 1;
                }
                else
                {
                    /* display the first digit of the temperature. */
                    SevenSegment_Display(g_u8Heater_desired_temperature % 10 ,
                                         SEVEN_SEGMENT_NOT_DISPLAY_DOT);
                    /* Enable Seven segment number 3 only. */
                    SevenSegment_SelectChip(SEVEN_SEGMENT_DISABLE_CHIP ,
                                            SEVEN_SEGMENT_DISABLE_CHIP ,
                                            SEVEN_SEGMENT_ENABLE_CHIP ,
                                            SEVEN_SEGMENT_DISABLE_CHIP);
                    g_u8Sync_flag = 0;
                }
            }
            else if(g_u8Display_blank_counter == 10)
            {
                /* reset synchronization flag between two Seven Segments. */
                g_u8Sync_flag = 0;
            }
            else if(g_u8Display_blank_counter == 20)
            {
                g_u8Display_blank_counter = 0;
            }
            else
            {
                /* Do Nothing */
            }
            break;
        default:
            /* Do Nothing. */
            break;
    }
}

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
void Heater_On_Off_Button_Task(void)
{
    /* buffer to save push button state. */
    static EnumButton_State_t On_Off_Button;
    /* buffer to get saved temperature from EEPROM. */
    uint8 u8Saved_temperature_in_EEPROM;
    /* Get the current state of on/off push button. */
    Check_ON_Push_Button(ON_OFF_PORT , ON_OFF_PIN , &On_Off_Button);
    switch(g_Heater_state_t)
    {
        case HEATER_OFF_STATE:
            if(On_Off_Button == PUSH_BUTTON_IS_PRESSED)
            {
                /* Get saved temperature from EEPROM if it between 35 and 75, else make the user temperature equal 60 degree. */
                u8Saved_temperature_in_EEPROM = EEPROM_Read_Byte(EEPROM_LOCATION_FOR_SAVED_TEMP);
                /* check if the saved temperature in the range of heater temperature. */
                g_u8Heater_desired_temperature = (u8Saved_temperature_in_EEPROM >= 35 &&
                                                 u8Saved_temperature_in_EEPROM <= 75) ? 
                                                 u8Saved_temperature_in_EEPROM : 60;
                /* Change the device state to heating process to start heating. */
                g_Heater_state_t = HEATER_ON_STATE;
            }
            else if(On_Off_Button == PUSH_BUTTON_IS_NOT_PRESSED)
            {
                /* Sleep CPU to save the Power. */
                CPU_sleep();
            }
            else
            {
                /* Do Nothing. */
            }
            break;
        case HEATER_ON_STATE:
        case HEATER_SET_TEMERATURE:
            if(On_Off_Button == PUSH_BUTTON_IS_PRESSED)
            {
                Heater_turn_off();
            }
            else
            {
                /* Do nothing */
            }
            break;
        default:
            /* Do Nothing. */
            break;
    }
}

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
void Heater_Up_Down_Buttons_Task(void)
{
    /* buffers to save push button state. */
    static EnumButton_State_t Up_Button;
    static EnumButton_State_t Down_Button;
    switch(g_Heater_state_t)
    {
        case HEATER_OFF_STATE:
            /* Do Nothing. */
            break;
        case HEATER_ON_STATE:
            /* check if the user want to update heating temperature while the heating process. */
            if(Check_ON_Push_Button(DOWN_PORT , DOWN_PIN , &Down_Button) == PUSH_BUTTON_IS_PRESSED ||
               Check_ON_Push_Button(UP_PORT , UP_PIN , &Up_Button) == PUSH_BUTTON_IS_PRESSED)
            {
                /* change the device state to setup state. */
                g_Heater_state_t = HEATER_SET_TEMERATURE;
            }
            else
            {
                /* Do nothing */
            }
            break;
        case HEATER_SET_TEMERATURE:
            /* Check if the Down button was pressed to decrease the temperature. */
            if(Check_ON_Push_Button(DOWN_PORT , DOWN_PIN , &Down_Button) == PUSH_BUTTON_IS_PRESSED)
            {
                /* reset the flag that make sure the 5 seconds pass without press any buttons to return to heating state. */
                g_u8End_setup_state = 0;
                /* reset the flag that make sure the temperature blank every one second at setup state. */
                g_u8Display_blank_counter = 0;
                /* check if the user input temperature less than the boundary temperature or not. */
                if(g_u8Heater_desired_temperature > MIN_TEMPERATURE)
                {
                    /* decrease the heating temperature with 5 degrees. */
                    g_u8Heater_desired_temperature -= 5;
                    /* Save last temperature on EEPROM to use it in the next using process when the heater will be turned on. */
                    EEPROM_Save_Byte(EEPROM_LOCATION_FOR_SAVED_TEMP , g_u8Heater_desired_temperature);
                }
                else
                {
                    /* Do Nothing */
                }
            }
            /* Check if the Up button was pressed to increase the temperature. */
            else if(Check_ON_Push_Button(UP_PORT , UP_PIN , &Up_Button) == PUSH_BUTTON_IS_PRESSED)
            {
                /* reset the flag that make sure the 5 seconds pass without press any buttons to return to heating state. */
                g_u8End_setup_state = 0;
                /* reset the flag that make sure the temperature blank every one second at setup state. */
                g_u8Display_blank_counter = 0;
                /* check if the user input temperature greater than the boundary temperature or not. */
                if(g_u8Heater_desired_temperature < MAX_TEMPERATURE)
                {
                    /* increase the heating temperature with 5 degrees. */
                    g_u8Heater_desired_temperature += 5;
                    /* Save last temperature on EEPROM to use it in the next using process when the heater will be turned on. */
                    EEPROM_Save_Byte(EEPROM_LOCATION_FOR_SAVED_TEMP , g_u8Heater_desired_temperature);
                }
                else
                {
                    /* Do Nothing */
                }
            }
            else
            {
                /* Do nothing */
            }
            break;
        default:
            /* Do Nothing. */
            break;
    }
}

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
void Heater_LED_Task(void)
{
    switch(g_Heater_state_t)
    {
        case HEATER_OFF_STATE:
            /* Do Nothing. */
            break;
        case HEATER_ON_STATE:
            /* check if the average of the last ten readings of temperature sensor is greater than the wanted temperature. */
            if(g_u8Heater_desired_temperature < g_u8Heater_real_time_temperature)
            {
                /* turn on the LED referring to the cooling process. */
                GPIO_WriteOnPin(LEDS_PORT , LED5_PIN , ENABLE);
                /* initialize the flag that will be used to blank the LED at heating process. */
                g_u8Led_blank_counter = 20;
            }
            /* check if the average of the last ten readings of temperature sensor is less than the wanted temperature. */
            else if(g_u8Heater_desired_temperature > g_u8Heater_real_time_temperature)
            {
                /* check of the blank counter to blank the LED every 1 second. */
                if(g_u8Led_blank_counter == 10)
                {
                    /* turn the LED on. */
                    GPIO_WriteOnPin(LEDS_PORT , LED5_PIN , ENABLE);
                    /* increment blank flag. */
                    g_u8Led_blank_counter++;

                }
                else if(g_u8Led_blank_counter == 20)
                {
                    /* turn the LED off. */
                    GPIO_WriteOnPin(LEDS_PORT , LED5_PIN , DISABLE);
                    /* reset the blank flag. */ 
                    g_u8Led_blank_counter = 0;
                }
                else
                {
                    /* increment blank flag. */
                    g_u8Led_blank_counter++;
                }
            }
            /* the average of the last ten readings of temperature sensor is equal the wanted temperature. */
            else
            {
                /* Do Nothing. */
            }
            break;
        case HEATER_SET_TEMERATURE:
            /* turn the LED off. */
            GPIO_WriteOnPin(LEDS_PORT , LED5_PIN , DISABLE);
            /* reset the blank flag. */ 
            g_u8Led_blank_counter = 0;
            break;
        default:
            /* Do Nothing. */
            break;
    }
}

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
void Heater_Heating_Cooling_Task(void)
{
    switch(g_Heater_state_t)
    {
        case HEATER_OFF_STATE:
            /* Do Nothing. */
            break;
        case HEATER_ON_STATE:
            /* check if the average of the last ten readings of temperature sensor is greater than the wanted temperature. */
            if(g_u8Heater_desired_temperature < g_u8Heater_real_time_temperature)
            {
                /* Enable Cooler. */
                GPIO_WriteOnPin(COOLER_PORT , COOLER_PIN , ENABLE);
                /* Disable heater. */
                GPIO_WriteOnPin(HEATING_RESISTANCE_PORT , HEATING_RESISTANCE_PIN , DISABLE);
            }
            /* check if the average of the last ten readings of temperature sensor is less than the wanted temperature. */
            else if(g_u8Heater_desired_temperature > g_u8Heater_real_time_temperature)
            {
                /* Disable Cooler. */
                GPIO_WriteOnPin(COOLER_PORT , COOLER_PIN , DISABLE);
                /* Enable heater. */
                GPIO_WriteOnPin(HEATING_RESISTANCE_PORT , HEATING_RESISTANCE_PIN , ENABLE);
            }
            /* the average of the last ten readings of temperature sensor is equal the wanted temperature. */
            else
            {
                /* Disable Cooler. */
                GPIO_WriteOnPin(COOLER_PORT , COOLER_PIN , DISABLE);
                /* Disable heater. */
                GPIO_WriteOnPin(HEATING_RESISTANCE_PORT , HEATING_RESISTANCE_PIN , DISABLE);
            }
            break;
        case HEATER_SET_TEMERATURE:
            /* Disable Cooler. */
            GPIO_WriteOnPin(COOLER_PORT , COOLER_PIN , DISABLE);
            /* Disable heater. */
            GPIO_WriteOnPin(HEATING_RESISTANCE_PORT , HEATING_RESISTANCE_PIN , DISABLE);
            break;
        default:
            /* Do Nothing. */
            break;
    }
}

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
void Heater_Reading_Temperature_Sensor_Task(void)
{
    /* buffer to get the temperature sensor reading. */
    uint8 g_u8Current_reading_temperature;
    switch(g_Heater_state_t)
    {
        case HEATER_OFF_STATE:
            /* Do Nothing. */
            break;
        case HEATER_ON_STATE:
            /* start ADC sampling to read temperature sensor reading. */
            ADC_Start_conversion();
            /* get the temperature from ADC channel. */
            g_u8Current_reading_temperature = ADC_Read_value() / 2;
            /* check on the temperature to make it suitable with showing temperature on the PICSimulation program. (issue in the Simulation program) */
            g_u8Current_reading_temperature = g_u8Current_reading_temperature > 40 ?
                                              g_u8Current_reading_temperature - 1 : g_u8Current_reading_temperature;
            /* check if the sensor reading less than 10 readings to get the average. */
            if(g_u8Reading_counter < 10)
            {
                /* add the current sample to the sum of last samples. */
                g_u16Sum_of_ten_readings += g_u8Current_reading_temperature;
                /* add the reading to the array of the last 10 readings. */
                g_u8Reading_arr[g_u8Reading_counter] = g_u8Current_reading_temperature;
                /* increment the counter. */
                g_u8Reading_counter++;
                /* make real time temperature equal the current sensor reading. */
                g_u8Heater_real_time_temperature = g_u8Heater_desired_temperature;
            }
            /* the sensor sampled 10 samples or more. */
            else
            {
                /* add the new reading to the summation and remove the tenth oldest one. */
                g_u16Sum_of_ten_readings = g_u16Sum_of_ten_readings + g_u8Current_reading_temperature -
                                           g_u8Reading_arr[g_u8Reading_deleted_element];
                /* replace the tenth oldest sample with the current reading. */
                g_u8Reading_arr[g_u8Reading_deleted_element] = g_u8Current_reading_temperature;
                /* check if the array get its limit to return to the first element in the array of readings. */
                if(g_u8Reading_deleted_element == 9)
                {
                    g_u8Reading_deleted_element = 0;
                }
                else
                {
                    /* increment the index of the next deleted reading. */
                    g_u8Reading_deleted_element++;
                }
                /* Get the temperature of water based on average of last 10 readings. */
                g_u8Heater_real_time_temperature = g_u16Sum_of_ten_readings / 10;
            }
            break;
        case HEATER_SET_TEMERATURE:
            /* increment this flag to blank Seven Segment every second on setup state. */
            g_u8Display_blank_counter++;
            /* increment this flag to return to heating state after 5 seconds without
             *  pressing on any bush button. */
            g_u8End_setup_state++;
            /* check if the 5 seconds passed to change the state from setup state to
             *  heating state. */
            if(g_u8End_setup_state == 50)
            {
                /* reset blank counter for next setup operation. */
                g_u8Display_blank_counter = 0;
                /* Reset End flag that return from setup state after 5 seconds without press any Button. */
                g_u8End_setup_state = 0;
                /* Change the state from setup state to heating state. */
                g_Heater_state_t = HEATER_ON_STATE;
            }
            else
            {
                /* Do Nothing */
            }
            break;
        default:
            /* Do Nothing. */
            break;
    }
}
# 1 "main.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 288 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "C:\\Program Files (x86)\\Microchip\\xc8\\v2.10\\pic\\include\\language_support.h" 1 3
# 2 "<built-in>" 2
# 1 "main.c" 2
# 16 "main.c"
# 1 "./heater.h" 1
# 22 "./heater.h"
# 1 "./GPIO.h" 1
# 21 "./GPIO.h"
# 1 "./stdtypes.h" 1
# 52 "./stdtypes.h"
typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short int uint16;
typedef signed short int sint16;
typedef unsigned long int uint32;
typedef signed long int sint32;
typedef unsigned long long int uint64;
typedef signed long long int sint64;
# 21 "./GPIO.h" 2
# 46 "./GPIO.h"
typedef enum {
    GPIO_CORRECT_SET,
    GPIO_UNCORRECT_SET
}EnumGPIO_Status_t;
# 68 "./GPIO.h"
extern EnumGPIO_Status_t GPIO_SetPinDirection(uint8 a_u8port , uint8 a_u8pin , uint8 a_u8direction);
# 82 "./GPIO.h"
extern EnumGPIO_Status_t GPIO_SetPortDirection(uint8 a_u8port , uint8 a_u8direction);
# 97 "./GPIO.h"
extern EnumGPIO_Status_t GPIO_WriteOnPin(uint8 a_u8port , uint8 a_u8pin , uint8 a_u8value);
# 111 "./GPIO.h"
extern EnumGPIO_Status_t GPIO_WriteOnPort(uint8 a_u8port , uint8 a_u8value);
# 125 "./GPIO.h"
extern uint8 GPIO_ReadFromPin(uint8 a_u8port , uint8 a_u8pin);
# 138 "./GPIO.h"
extern uint8 GPIO_ReadFromPort(uint8 a_u8port);
# 152 "./GPIO.h"
extern EnumGPIO_Status_t GPIO_TogglePin(uint8 a_u8port , uint8 a_u8pin);
# 165 "./GPIO.h"
extern EnumGPIO_Status_t GPIO_TogglePort(uint8 a_u8port);
# 178 "./GPIO.h"
extern void GPIO_Disable_Comparator_On_PORTA(void);
# 192 "./GPIO.h"
extern EnumGPIO_Status_t GPIO_Disable_ADC_On_Pins(uint8 a_u8port , uint8 a_u8pin);
# 205 "./GPIO.h"
extern void GPIO_Enable_Pull_Up_On_PortB(void);
# 218 "./GPIO.h"
extern void GPIO_Disable_Pull_Up_On_PortB(void);
# 22 "./heater.h" 2


# 1 "./CPU_sleep.h" 1
# 33 "./CPU_sleep.h"
extern void CPU_sleep(void);
# 24 "./heater.h" 2

# 1 "./heater_config.h" 1
# 25 "./heater.h" 2
# 46 "./heater.h"
void Heater_Initialization(void);
# 59 "./heater.h"
void Heater_Display_Task(void);
# 72 "./heater.h"
void Heater_On_Off_Button_Task(void);
# 86 "./heater.h"
void Heater_Up_Down_Buttons_Task(void);
# 100 "./heater.h"
void Heater_LED_Task(void);
# 114 "./heater.h"
void Heater_Heating_Cooling_Task(void);
# 128 "./heater.h"
void Heater_Reading_Temperature_Sensor_Task(void);
# 16 "main.c" 2

# 1 "./scheduler.h" 1
# 21 "./scheduler.h"
# 1 "./scheduler_config.h" 1
# 21 "./scheduler.h" 2








typedef char TickTime;


typedef enum {
 ONCE,
 REPEAT
}EnumScheduler_RepeatedFunction_t;


typedef enum {
 SCHEDULER_NOT_AVAILABLE_TICK_TIME_ERROR,
    SCHEDULER_TASKS_QUEUE_IS_FULL_ERROR,
    SCHEDULER_ID_IS_NOT_EXIST_ERROR,
    SCHEDULER_NO_ERROR
}EnumScheduler_Error_t;
# 60 "./scheduler.h"
extern EnumScheduler_Error_t Scheduler_Initialization(TickTime a_u8tickTme);
# 77 "./scheduler.h"
extern EnumScheduler_Error_t Scheduler_StartFunction(void (*task_function_ptr) (void),
        uint8 a_u8task_ID , uint16 a_u16task_periodicity , uint16 a_u16task_start_time ,
                                            EnumScheduler_RepeatedFunction_t a_RepeatType_t);
# 92 "./scheduler.h"
extern EnumScheduler_Error_t Scheduler_StopFunction(uint8 a_u8_task_ID);
# 105 "./scheduler.h"
extern void Scheduler_Dispatcher(void);
# 17 "main.c" 2





void initialization_Tasks(void);


void main(void)
{

    Heater_Initialization();

    Scheduler_Initialization(1u);

    Scheduler_StartFunction(initialization_Tasks , 1 , 1 , 0 , ONCE);

    while(1)
    {

        Scheduler_Dispatcher();
    }
}
# 52 "main.c"
void initialization_Tasks(void)
{

    Scheduler_StartFunction(Heater_On_Off_Button_Task , 1 , 10 , 0 , REPEAT);

    Scheduler_StartFunction(Heater_Up_Down_Buttons_Task , 2 , 10 , 1 , REPEAT);

    Scheduler_StartFunction(Heater_Display_Task , 3 , 10 , 3 , REPEAT);

    Scheduler_StartFunction(Heater_Reading_Temperature_Sensor_Task , 4 , 100 , 5 , REPEAT);

    Scheduler_StartFunction(Heater_Heating_Cooling_Task , 5 , 100 , 6 , REPEAT);

    Scheduler_StartFunction(Heater_LED_Task , 6 , 100 , 7 , REPEAT);
}

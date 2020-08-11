# 1 "scheduler.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 288 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "C:\\Program Files (x86)\\Microchip\\xc8\\v2.10\\pic\\include\\language_support.h" 1 3
# 2 "<built-in>" 2
# 1 "scheduler.c" 2
# 16 "scheduler.c"
# 1 "./scheduler.h" 1
# 21 "./scheduler.h"
# 1 "./scheduler_config.h" 1
# 21 "./scheduler.h" 2

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
# 22 "./scheduler.h" 2







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
# 16 "scheduler.c" 2

# 1 "./timers.h" 1
# 21 "./timers.h"
# 1 "./timers_config.h" 1
# 21 "./timers.h" 2
# 124 "./timers.h"
typedef enum{
            TIMER1_INTERNAL_CLOCK,
            TIMER1_TRANSITION_ON_TCKI_PIN
}EnumTimer1_clockSource_t;


typedef enum{
            TIMER1_NO_PRESCALER,
            TIMER1_PRESCALER_2,
            TIMER1_PRESCALER_4,
            TIMER1_PRESCALER_8
}EnumTimer1_prescaler_t;


typedef enum{
            TIMER1_SYNC_WITH_EXTERNAL_CLOCK,
            TIMER1_NOT_SYNC_WITH_EXTERNAL_CLOCK,
            TIMER1_NO_EXTERNAL_CLOCK
}EnumTimer1_sync_with_external_clock_t;


typedef struct{
            EnumTimer1_prescaler_t timer_prescaler_t;
            EnumTimer1_clockSource_t timer_clock_source_t;
            EnumTimer1_sync_with_external_clock_t timer_sync_with_external_clock;
}StrTimer1_config_t;





extern StrTimer1_config_t Timer1_config_t;
# 173 "./timers.h"
extern void Timer1_Initialization(void);
# 186 "./timers.h"
extern void Timer1_enable(void);
# 199 "./timers.h"
extern void Timer1_disable(void);
# 212 "./timers.h"
extern uint16 Timer1_read_counter(void);
# 226 "./timers.h"
extern void Timer1_write_counter(uint16 a_u16init_value_for_timer);
# 239 "./timers.h"
extern void Timer1_oscillator_enable(void);
# 252 "./timers.h"
extern void Timer1_oscillator_disable(void);
# 17 "scheduler.c" 2

# 1 "./interrupt.h" 1
# 22 "./interrupt.h"
# 1 "./interrupt_config.h" 1
# 22 "./interrupt.h" 2
# 40 "./interrupt.h"
extern void Global_interrupt_enable(void);
# 53 "./interrupt.h"
extern void Global_interrupt_disable(void);
# 67 "./interrupt.h"
extern void Peripherals_interrupt_enable(void);
# 81 "./interrupt.h"
extern void Peripherals_interrupt_disable(void);
# 140 "./interrupt.h"
extern void Timer1_enable_overflow_interrupt(void);
# 153 "./interrupt.h"
extern void Timer1_disable_overflow_interrupt(void);
# 166 "./interrupt.h"
extern void Timer1_set_callback_function(void(*a_function_ptr)(void));
# 226 "./interrupt.h"
extern void External_interrupt_enable(void);
# 239 "./interrupt.h"
extern void External_interrupt_disable(void);
# 252 "./interrupt.h"
extern void External_interrupt_select_rising_edge(void);
# 265 "./interrupt.h"
extern void External_interrupt_select_falling_edge(void);
# 278 "./interrupt.h"
extern void External_interrupt_set_callback_function(void(*a_function_ptr)(void));
# 18 "scheduler.c" 2

# 1 "./CPU_sleep.h" 1
# 33 "./CPU_sleep.h"
extern void CPU_sleep(void);
# 19 "scheduler.c" 2






typedef enum {
 READY,
 RUNNING,
 WAITING
}EnumScheduler_Status_t;


typedef struct {
 uint8 Task_ID;
 uint16 Task_ExecutionTime;
 uint16 Task_Periodicity;
 EnumScheduler_RepeatedFunction_t Task_RepeatType;
 EnumScheduler_Status_t Task_State;
 void (*Task_Function)(void);
}StructScheduler_Object_t;





volatile static uint16 g_u16_TickTimeCounter = 0;

static StructScheduler_Object_t Scheduler_Tasks[10u];

uint32 u32compare_value;
# 71 "scheduler.c"
static void Scheduler_Core(void)
{

 g_u16_TickTimeCounter++;

    Timer1_write_counter(65535u - u32compare_value);

 for(uint16 Counter = 0 ; Counter < 10u ; Counter++)
 {

  if((Scheduler_Tasks[Counter].Task_ExecutionTime == g_u16_TickTimeCounter)
    && Scheduler_Tasks[Counter].Task_ID != 0 )
  {

   Scheduler_Tasks[Counter].Task_State = READY;

            Scheduler_Tasks[Counter].Task_ExecutionTime += Scheduler_Tasks[Counter].Task_Periodicity;
  }
  else
  {

  }
 }
}
# 108 "scheduler.c"
static EnumScheduler_Error_t Scheduler_Tick_time_calculation(TickTime a_u8tickTme)
{

    EnumScheduler_Error_t tick_calculation_t = SCHEDULER_NO_ERROR;

    u32compare_value = (8000000u / 4000) * a_u8tickTme;

    if(u32compare_value < 65535u)
    {
        Timer1_config_t.timer_prescaler_t = TIMER1_NO_PRESCALER;

        Timer1_Initialization();
        tick_calculation_t = SCHEDULER_NO_ERROR;
    }
    else if (u32compare_value / 2 < 65535u)
    {
        Timer1_config_t.timer_prescaler_t = TIMER1_PRESCALER_2;
        u32compare_value /= 2;

        Timer1_Initialization();
        tick_calculation_t = SCHEDULER_NO_ERROR;
    }
    else if (u32compare_value / 4 < 65535u)
    {
        Timer1_config_t.timer_prescaler_t = TIMER1_PRESCALER_4;
        u32compare_value /= 4;

        Timer1_Initialization();
        tick_calculation_t = SCHEDULER_NO_ERROR;
    }
    else if (u32compare_value / 8 < 65535u)
    {
        Timer1_config_t.timer_prescaler_t = TIMER1_PRESCALER_8;
        u32compare_value /= 8;

        Timer1_Initialization();
        tick_calculation_t = SCHEDULER_NO_ERROR;
    }
    else
    {
        tick_calculation_t = SCHEDULER_NOT_AVAILABLE_TICK_TIME_ERROR;
    }
    return tick_calculation_t;
}
# 167 "scheduler.c"
EnumScheduler_Error_t Scheduler_Initialization(TickTime a_u8tickTme)
{

    EnumScheduler_Error_t initialization_error_t = SCHEDULER_NO_ERROR;

    initialization_error_t = Scheduler_Tick_time_calculation(a_u8tickTme);

    if(initialization_error_t == SCHEDULER_NO_ERROR)
    {

        Timer1_set_callback_function(Scheduler_Core);

        Global_interrupt_enable();

        Peripherals_interrupt_enable();

        for(uint16 Counter = 0 ; Counter < 10u ; Counter++)
        {
            Scheduler_Tasks[Counter].Task_ID = 0;
            Scheduler_Tasks[Counter].Task_Periodicity = 0;
            Scheduler_Tasks[Counter].Task_RepeatType = ONCE;
            Scheduler_Tasks[Counter].Task_ExecutionTime = 0;
            Scheduler_Tasks[Counter].Task_State = WAITING;
            Scheduler_Tasks[Counter].Task_Function = (void*)0x00;
        }

        Timer1_enable();
    }
    else
    {
        initialization_error_t = SCHEDULER_NOT_AVAILABLE_TICK_TIME_ERROR;
    }
    return initialization_error_t;
}
# 217 "scheduler.c"
EnumScheduler_Error_t Scheduler_StartFunction(void (*task_function_ptr) (void), uint8 a_u8_task_ID ,
  uint16 a_u16_task_periodicity , uint16 a_u16_task_start_time ,
        EnumScheduler_RepeatedFunction_t Enum_Task_RepeatType)
{

    EnumScheduler_Error_t starting_task_error = SCHEDULER_NO_ERROR;

 Timer1_enable_overflow_interrupt();

 for(uint16 Counter = 0 ; Counter < 10u ; Counter++)
 {
  if(Scheduler_Tasks[Counter].Task_ID == 0)
  {
   Scheduler_Tasks[Counter].Task_ID = a_u8_task_ID;
   Scheduler_Tasks[Counter].Task_Periodicity = a_u16_task_periodicity;
   Scheduler_Tasks[Counter].Task_RepeatType = Enum_Task_RepeatType;
   Scheduler_Tasks[Counter].Task_ExecutionTime = a_u16_task_start_time + g_u16_TickTimeCounter + 1;
   Scheduler_Tasks[Counter].Task_State = WAITING;
   Scheduler_Tasks[Counter].Task_Function = task_function_ptr;
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
# 258 "scheduler.c"
EnumScheduler_Error_t Scheduler_StopFunction(uint8 a_u8_task_ID)
{

    EnumScheduler_Error_t stoping_task_error = SCHEDULER_NO_ERROR;

 for(uint16 Counter=0;Counter<10u;Counter++)
 {
  if(Scheduler_Tasks[Counter].Task_ID == a_u8_task_ID)
  {
   Scheduler_Tasks[Counter].Task_ID = 0;
   Scheduler_Tasks[Counter].Task_Periodicity = 0;
   Scheduler_Tasks[Counter].Task_RepeatType = ONCE;
   Scheduler_Tasks[Counter].Task_ExecutionTime = 0;
   Scheduler_Tasks[Counter].Task_State = WAITING;
   Scheduler_Tasks[Counter].Task_Function = (void*)0x00;
            stoping_task_error = SCHEDULER_NO_ERROR;
   break;
  }
  else
  {
   stoping_task_error = SCHEDULER_ID_IS_NOT_EXIST_ERROR;
  }
 }

 uint16 Counter = 0 ;

 for(; Counter < 10u ; Counter++)
 {
  if(Scheduler_Tasks[Counter].Task_ID != 0)
  {
   break;
  }
  else
  {

  }
 }

 if(Counter == 10u)
 {

  Timer1_disable_overflow_interrupt();
 }
 else
 {

 }
    return stoping_task_error;
}
# 319 "scheduler.c"
void Scheduler_Dispatcher(void)
{

 for(uint16 Counter = 0 ; Counter < 10u ; Counter++)
 {

  if(Scheduler_Tasks[Counter].Task_State == READY)
  {
   Scheduler_Tasks[Counter].Task_State = RUNNING;
   Scheduler_Tasks[Counter].Task_Function();
   Scheduler_Tasks[Counter].Task_State = WAITING;

            if(Scheduler_Tasks[Counter].Task_RepeatType == ONCE)
            {

                Scheduler_Tasks[Counter].Task_ID = 0;
                Scheduler_Tasks[Counter].Task_Periodicity = 0;
                Scheduler_Tasks[Counter].Task_RepeatType = ONCE;
                Scheduler_Tasks[Counter].Task_ExecutionTime = 0;
                Scheduler_Tasks[Counter].Task_State = WAITING;
                Scheduler_Tasks[Counter].Task_Function = (void*)0x00;
            }
            else
            {

            }
  }
  else
  {

  }
 }
}

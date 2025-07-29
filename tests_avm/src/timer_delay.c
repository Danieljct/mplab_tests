/******************************************************************************
* Title                 :   Global Timer Delay System
* Filename              :   timer_delay.c
* Author                :   Daniel
* Origin Date           :   28/07/2025
* Version               :   1.0.0
* Target                :   SAMD51J20A
* Notes                 :   Global timer-based delay system using TC1
*
******************************************************************************/

/******************************************************************************
**      HEADER FILES
******************************************************************************/
#include "timer_delay.h"
#include "definitions.h"
#include "peripheral/tc/plib_tc1.h"

/******************************************************************************
**      MODULE PREPROCESSOR CONSTANTS
******************************************************************************/

/******************************************************************************
**      MODULE MACROS
******************************************************************************/

/******************************************************************************
*       MODULE DATATYPES
******************************************************************************/

/******************************************************************************
**      MODULE VARIABLE DEFINITIONS
******************************************************************************/
static volatile uint32_t timerDelayCounter = 0;
static volatile uint32_t nonBlockingDelayCounter = 0;
static bool timerDelayInitialized = false;

/******************************************************************************
**      PRIVATE FUNCTION DECLARATIONS (PROTOTYPES)
******************************************************************************/
static void TimerDelay_Callback(uint32_t status, uintptr_t context);

/******************************************************************************
**      FUNCTION DEFINITIONS
******************************************************************************/

/**
 * @brief Initialize the global timer delay system
 */
TimerDelayResult_t TimerDelay_Init(void)
{
    if (timerDelayInitialized)
    {
        return TIMER_DELAY_ERROR_ALREADY_INIT;
    }
    
    // Initialize TC1 timer
    TC1_TimerInitialize();
    
    // Register callback for timer interrupt (every 1ms)
    TC1_TimerCallbackRegister(TimerDelay_Callback, (uintptr_t)NULL);
    
    // Start the timer
    TC1_TimerStart();
    
    // Reset counters
    timerDelayCounter = 0;
    nonBlockingDelayCounter = 0;
    
    timerDelayInitialized = true;
    return TIMER_DELAY_SUCCESS;
}

/**
 * @brief Check if timer delay system is initialized
 */
bool TimerDelay_IsInitialized(void)
{
    return timerDelayInitialized;
}

/**
 * @brief Precise blocking delay function using timer (1ms resolution)
 */
TimerDelayResult_t TimerDelay_Ms(uint32_t ms)
{
    // Auto-initialize if not done
    if (!timerDelayInitialized)
    {
        TimerDelayResult_t result = TimerDelay_Init();
        if (result != TIMER_DELAY_SUCCESS)
        {
            return result;
        }
    }
    
    if (ms == 0)
        return TIMER_DELAY_SUCCESS;
    
    // Set delay counter
    timerDelayCounter = ms;
    
    // Wait until counter reaches zero
    while (timerDelayCounter > 0)
    {
        // Blocking wait - could add yield here if using RTOS
    }
    
    return TIMER_DELAY_SUCCESS;
}

/**
 * @brief Start non-blocking delay
 */
TimerDelayResult_t TimerDelay_StartNonBlocking(uint32_t ms)
{
    // Auto-initialize if not done
    if (!timerDelayInitialized)
    {
        TimerDelayResult_t result = TimerDelay_Init();
        if (result != TIMER_DELAY_SUCCESS)
        {
            return result;
        }
    }
    
    nonBlockingDelayCounter = ms;
    return TIMER_DELAY_SUCCESS;
}

/**
 * @brief Check if non-blocking delay is finished
 */
bool TimerDelay_IsFinished(void)
{
    return (nonBlockingDelayCounter == 0);
}

/**
 * @brief Get remaining delay time
 */
uint32_t TimerDelay_GetRemaining(void)
{
    return nonBlockingDelayCounter;
}

/**
 * @brief Stop current delay operation
 */
void TimerDelay_Stop(void)
{
    timerDelayCounter = 0;
    nonBlockingDelayCounter = 0;
}

/**
 * @brief Deinitialize the timer delay system
 */
void TimerDelay_Deinit(void)
{
    if (timerDelayInitialized)
    {
        TC1_TimerStop();
        timerDelayCounter = 0;
        nonBlockingDelayCounter = 0;
        timerDelayInitialized = false;
    }
}

/******************************************************************************
**      PRIVATE FUNCTION DEFINITIONS
******************************************************************************/

/**
 * @brief Timer callback function - called every 1ms
 */
static void TimerDelay_Callback(uint32_t status, uintptr_t context)
{
    // Decrement blocking delay counter
    if (timerDelayCounter > 0)
    {
        timerDelayCounter--;
    }
    
    // Decrement non-blocking delay counter
    if (nonBlockingDelayCounter > 0)
    {
        nonBlockingDelayCounter--;
    }
}

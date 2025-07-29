/******************************************************************************
* Title                 :   Global Timer Delay System
* Filename              :   timer_delay.h
* Author                :   Daniel
* Origin Date           :   28/07/2025
* Version               :   1.0.0
* Target                :   SAMD51J20A
* Notes                 :   Global timer-based delay system using TC1
*
******************************************************************************/

#ifndef TIMER_DELAY_H_
#define TIMER_DELAY_H_

/******************************************************************************
**      INCLUDES
******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/******************************************************************************
**      CONFIGURATION CONSTANTS
******************************************************************************/

/******************************************************************************
**      MACRO DEFINITIONS
******************************************************************************/

/*****************************************************************************
**      PUBLIC DATATYPES														
*****************************************************************************/
typedef enum
{
    TIMER_DELAY_SUCCESS = 0,
    TIMER_DELAY_ERROR_INIT,
    TIMER_DELAY_ERROR_ALREADY_INIT,
    TIMER_DELAY_ERROR_TIMEOUT
} TimerDelayResult_t;

/*****************************************************************************
**      PUBLIC (GLOBAL) VARIABLES DECLARATIONS
*****************************************************************************/

/******************************************************************************
**      PUBLIC FUNCTION PROTOTYPES
******************************************************************************/

/**
 * @brief Initialize the global timer delay system
 * @return TIMER_DELAY_SUCCESS if successful, error code otherwise
 */
TimerDelayResult_t TimerDelay_Init(void);

/**
 * @brief Check if timer delay system is initialized
 * @return true if initialized, false otherwise
 */
bool TimerDelay_IsInitialized(void);

/**
 * @brief Precise delay function using timer (1ms resolution)
 * @param ms Delay in milliseconds
 * @return TIMER_DELAY_SUCCESS if successful, error code otherwise
 */
TimerDelayResult_t TimerDelay_Ms(uint32_t ms);

/**
 * @brief Non-blocking delay start
 * @param ms Delay in milliseconds
 * @return TIMER_DELAY_SUCCESS if successful, error code otherwise
 */
TimerDelayResult_t TimerDelay_StartNonBlocking(uint32_t ms);

/**
 * @brief Check if non-blocking delay is finished
 * @return true if delay is finished, false if still counting
 */
bool TimerDelay_IsFinished(void);

/**
 * @brief Get remaining delay time
 * @return Remaining time in milliseconds
 */
uint32_t TimerDelay_GetRemaining(void);

/**
 * @brief Stop current delay operation
 */
void TimerDelay_Stop(void);

/**
 * @brief Deinitialize the timer delay system
 */
void TimerDelay_Deinit(void);

#ifdef __cplusplus
}
#endif

#endif // TIMER_DELAY_H_

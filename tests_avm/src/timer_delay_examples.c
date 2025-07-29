/******************************************************************************
* Title                 :   Timer Delay Usage Examples
* Filename              :   timer_delay_examples.c
* Author                :   Generated for MPLAB Harmony Framework
* Origin Date           :   28/07/2025
* Version               :   1.0.0
* Target                :   SAMD51J20A
* Notes                 :   Examples of how to use the global timer delay system
*
******************************************************************************/

/******************************************************************************
**      HEADER FILES
******************************************************************************/
#include "timer_delay.h"
#include "definitions.h"

/******************************************************************************
**      EXAMPLE FUNCTIONS
******************************************************************************/

/**
 * @brief Example 1: Simple blocking delay
 */
void Example_SimpleDelay(void)
{
    // Initialize timer delay system (optional - auto-initializes)
    TimerDelay_Init();
    
    // Simple 1 second delay
    TimerDelay_Ms(1000);
    
    // Continue with your code...
}

/**
 * @brief Example 2: Non-blocking delay for state machines
 */
void Example_NonBlockingDelay(void)
{
    static enum {
        STATE_INIT,
        STATE_WAITING,
        STATE_PROCESSING,
        STATE_DONE
    } state = STATE_INIT;
    
    switch(state)
    {
        case STATE_INIT:
            SYS_CONSOLE_PRINT("Starting operation...\r\n");
            TimerDelay_StartNonBlocking(2000); // 2 second delay
            state = STATE_WAITING;
            break;
            
        case STATE_WAITING:
            if (TimerDelay_IsFinished())
            {
                SYS_CONSOLE_PRINT("Delay finished, processing...\r\n");
                state = STATE_PROCESSING;
            }
            // Can do other tasks while waiting
            break;
            
        case STATE_PROCESSING:
            // Do your processing here
            SYS_CONSOLE_PRINT("Processing complete!\r\n");
            state = STATE_DONE;
            break;
            
        case STATE_DONE:
            // Reset for next cycle if needed
            state = STATE_INIT;
            break;
    }
}

/**
 * @brief Example 3: Timeout functionality
 */
bool Example_TimeoutOperation(void)
{
    const uint32_t TIMEOUT_MS = 5000; // 5 second timeout
    
    // Start timeout timer
    TimerDelay_StartNonBlocking(TIMEOUT_MS);
    
    // Simulate some operation that might take time
    while (/* some_condition_not_met */ true)
    {
        // Check if timeout occurred
        if (TimerDelay_IsFinished())
        {
            SYS_CONSOLE_PRINT("Operation timed out!\r\n");
            return false; // Timeout occurred
        }
        
        // Check your operation status here
        // For this example, we'll break after some time
        static uint32_t counter = 0;
        if (++counter > 1000000) // Simulate work completing
        {
            break;
        }
    }
    
    // Stop the timeout timer since operation completed
    TimerDelay_Stop();
    SYS_CONSOLE_PRINT("Operation completed successfully!\r\n");
    return true; // Success
}

/**
 * @brief Example 4: Periodic tasks using delays
 */
void Example_PeriodicTask(void)
{
    static uint32_t lastTime = 0;
    static bool initialized = false;
    
    if (!initialized)
    {
        TimerDelay_Init();
        TimerDelay_StartNonBlocking(0); // Start with 0 to get current "time"
        initialized = true;
    }
    
    // Check if 500ms have passed
    if (TimerDelay_IsFinished())
    {
        // Execute periodic task
        SYS_CONSOLE_PRINT("Periodic task executed!\r\n");
        
        // Restart timer for next period
        TimerDelay_StartNonBlocking(500);
    }
}

/**
 * @brief Example 5: Multiple independent delays
 */
void Example_MultipleDelays(void)
{
    // Note: The current implementation supports one non-blocking delay at a time
    // For multiple independent delays, you would need to extend the system
    // or use blocking delays in different contexts
    
    // This example shows how you might handle multiple timing requirements
    static uint32_t led_blink_timer = 0;
    static uint32_t sensor_read_timer = 0;
    static uint32_t display_update_timer = 0;
    
    // Simulate multiple timing requirements
    // (In a real implementation, you'd need multiple timer counters)
    
    SYS_CONSOLE_PRINT("Example: Multiple timing requirements handled\r\n");
}

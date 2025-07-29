/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <stdio.h>
#include "codec.h"                      // Codec driver
#include "timer_delay.h"                // Global timer delay system

static bool volatile bToggleLED = false;
static uint16_t volatile adcValue = 0;
static bool codecTestDone = false;

// This function is called after period expires
void TC0_CH0_TimerInterruptHandler(uint32_t status, uintptr_t context)
{
    bToggleLED = true;
}

// Function to test codec functionality
void CODEC_TestFunction(void)
{
    CodecGain_t codecGain;
    CodecResult_t result;
    uint8_t regValue;
    
    SYS_CONSOLE_PRINT("\r\n=== CODEC Test Started ===\r\n");
    
    // Initialize global timer delay system first (optional - it will be done automatically)
    SYS_CONSOLE_PRINT("Initializing Global Timer Delay System...\r\n");
    TimerDelayResult_t timerResult = TimerDelay_Init();
    if (timerResult == TIMER_DELAY_SUCCESS)
    {
        SYS_CONSOLE_PRINT("Global Timer Delay System initialized successfully!\r\n");
    }
    else if (timerResult == TIMER_DELAY_ERROR_ALREADY_INIT)
    {
        SYS_CONSOLE_PRINT("Global Timer Delay System was already initialized!\r\n");
    }
    else
    {
        SYS_CONSOLE_PRINT("Failed to initialize Timer Delay System!\r\n");
    }
    
    // Test the global delay system
    SYS_CONSOLE_PRINT("Testing global delay system (2 second delay)...\r\n");
    TimerDelay_Ms(2000);
    SYS_CONSOLE_PRINT("Delay test completed!\r\n");
    
    // Initialize codec gain settings
    codecGain.leftGain = 0x50;   // Example gain value
    codecGain.rightGain = 0x50;  // Example gain value  
    codecGain.micGain = 0x40;    // Example mic gain
    
    // Initialize codec
    SYS_CONSOLE_PRINT("Initializing CODEC...\r\n");
    result = CODEC_init(codecGain);
    
    if (result == CODEC_SUCCESS)
    {
        SYS_CONSOLE_PRINT("CODEC initialized successfully!\r\n");
        
        // Test writing and reading registers
        SYS_CONSOLE_PRINT("Testing register write/read...\r\n");
        
        // Write test value to sample rate register
        result = CODEC_writeRegister(CODEC_REG_SAMPLE_RATE_SELECT, 0x22);
        if (result == CODEC_SUCCESS)
        {
            SYS_CONSOLE_PRINT("Write to Sample Rate Register: SUCCESS\r\n");
            
            // Read back the value
            result = CODEC_readRegister(CODEC_REG_SAMPLE_RATE_SELECT, &regValue);
            if (result == CODEC_SUCCESS)
            {
                SYS_CONSOLE_PRINT("Read from Sample Rate Register: 0x%02X\r\n", regValue);
            }
            else
            {
                SYS_CONSOLE_PRINT("Read from Sample Rate Register: FAILED\r\n");
            }
        }
        else
        {
            SYS_CONSOLE_PRINT("Write to Sample Rate Register: FAILED\r\n");
        }
        
        // Test gain setting
        SYS_CONSOLE_PRINT("Testing gain configuration...\r\n");
        codecGain.leftGain = 0x60;
        codecGain.rightGain = 0x65;
        codecGain.micGain = 0x45;
        
        result = CODEC_setGain(codecGain);
        if (result == CODEC_SUCCESS)
        {
            SYS_CONSOLE_PRINT("Gain configuration: SUCCESS\r\n");
        }
        else
        {
            SYS_CONSOLE_PRINT("Gain configuration: FAILED\r\n");
        }
        
        // Print all register values
        CODEC_printAllRegisters();
        
        // Test non-blocking delay functionality
        SYS_CONSOLE_PRINT("Testing non-blocking delay (3 seconds)...\r\n");
        TimerDelay_StartNonBlocking(3000);
        
        while (!TimerDelay_IsFinished())
        {
            uint32_t remaining = TimerDelay_GetRemaining();
            if (remaining % 500 == 0) // Print every 500ms
            {
                SYS_CONSOLE_PRINT("Remaining: %d ms\r\n", remaining);
            }
            TimerDelay_Ms(10); // Small delay to avoid flooding console
        }
        SYS_CONSOLE_PRINT("Non-blocking delay test completed!\r\n");
    }
    else
    {
        SYS_CONSOLE_PRINT("CODEC initialization FAILED! Error code: %d\r\n", result);
    }
    
    SYS_CONSOLE_PRINT("=== CODEC Test Completed ===\r\n\r\n");
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    // Initialize all modules
    SYS_Initialize(NULL);

    // Enable ADC1
    ADC1_Enable();

    // Register callback function for CH0 period interrupt
    TC0_TimerCallbackRegister(TC0_CH0_TimerInterruptHandler, (uintptr_t)NULL);

    // Start the timer channel 0
    TC0_TimerStart();

    while ( true )
    {
        // Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks();
        
        // Run codec test once after system initialization
        if (!codecTestDone)
        {
            // Wait for system to stabilize using global timer delay (2 seconds)
            static bool delayStarted = false;
            
            if (!delayStarted)
            {
                // Initialize timer delay system if not already done
                TimerDelay_Init();
                // Start 2 second delay for system stabilization
                TimerDelay_StartNonBlocking(2000);
                delayStarted = true;
                SYS_CONSOLE_PRINT("System stabilizing... please wait 2 seconds\r\n");
            }
            
            if (TimerDelay_IsFinished())
            {
                CODEC_TestFunction();
                codecTestDone = true;
            }
        }
        
        // ADC reading logic
        ADC1_ConversionStart();
        while (!ADC1_ConversionStatusGet())
        {
            // Wait for conversion result
        }
        
        if ( bToggleLED )
        {
            // Read ADC value
            adcValue = ADC1_ConversionResultGet();
            SYS_CONSOLE_PRINT("ADC Value: %d\r\n", adcValue);
            bToggleLED = false;
            LED_R_Toggle();
        }
    }

    // Execution should not come here during normal operation
    return EXIT_FAILURE;
}

/*******************************************************************************
 End of File
*/

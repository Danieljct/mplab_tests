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
#include <stdio.h>                      // Defines sprintf
#include <string.h>                     // Defines strlen
#include "definitions.h"                // SYS function prototypes

static bool volatile bToggleLED = false;
static uint16_t volatile adcValue = 0;

// USB Variables
static USB_DEVICE_HANDLE usbDeviceHandle;
static USB_DEVICE_CDC_TRANSFER_HANDLE usbTransferHandle;
static bool usbDeviceConfigured = false;
static char logBuffer[256];

// This function is called after period expires
void TC0_CH0_TimerInterruptHandler(TC_TIMER_STATUS status, uintptr_t context)
{
    bToggleLED = true;
}

// USB CDC Event Handler
USB_DEVICE_CDC_EVENT_RESPONSE APP_USBDeviceCDCEventHandler(USB_DEVICE_CDC_INDEX index, USB_DEVICE_CDC_EVENT event, void* pData, uintptr_t userData)
{
    switch(event)
    {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:
        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:
        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:
            USB_DEVICE_ControlStatus(usbDeviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;
            
        default:
            break;
    }
    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}


// USB Device Event Handler
USB_DEVICE_EVENT_RESPONSE APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void* eventData, uintptr_t context)
{
    switch(event)
    {
        case USB_DEVICE_EVENT_CONFIGURED:
            usbDeviceConfigured = true;
            USB_DEVICE_CDC_EventHandlerSet(USB_DEVICE_CDC_INDEX_0, APP_USBDeviceCDCEventHandler, (uintptr_t)0);
            break;
            
        case USB_DEVICE_EVENT_DECONFIGURED:
            usbDeviceConfigured = false;
            break;
            
        case USB_DEVICE_EVENT_SUSPENDED:
            break;
            
        case USB_DEVICE_EVENT_RESUMED:
            break;
            
        case USB_DEVICE_EVENT_POWER_DETECTED:
            USB_DEVICE_Attach(usbDeviceHandle);
            break;
            
        case USB_DEVICE_EVENT_POWER_REMOVED:
            USB_DEVICE_Detach(usbDeviceHandle);
            break;
            
        default:
            break;
    }
    return USB_DEVICE_EVENT_RESPONSE_NONE;
}


// Function to send log via USB
void USB_SendLog(const char* message)
{
    if (usbDeviceConfigured)
    {
        USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0, &usbTransferHandle, message, strlen(message), USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
    }
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

    // Wait for system to stabilize
   // for(volatile int i = 0; i < 100000; i++);

    // Initialize USB Device with retry mechanism
    int retryCount = 0;
    do {
        usbDeviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);
        if (usbDeviceHandle != USB_DEVICE_HANDLE_INVALID) {
            break;
        }
        retryCount++;
        // Small delay between retries
        for(volatile int i = 0; i < 10000; i++);
    } while (retryCount < 10);

    if (usbDeviceHandle != USB_DEVICE_HANDLE_INVALID) {
        USB_DEVICE_EventHandlerSet(usbDeviceHandle, APP_USBDeviceEventHandler, 0);
    }

    // Enable ADC1
    ADC1_Enable();

    // Register callback function for CH0 period interrupt
    TC0_TimerCallbackRegister(TC0_CH0_TimerInterruptHandler, (uintptr_t)NULL);

    // Start the timer channel 0
    TC0_TimerStart();

    // Send initial log message
    USB_SendLog("Sistema iniciado - ADC1 habilitado\r\n");

    while ( true )
    {
        // Start ADC conversion
        ADC1_ConversionStart();
        
        // Wait for conversion to complete
        while (!ADC1_ConversionStatusGet())
        {
            // Wait for conversion result
        }

        if ( bToggleLED )
        {
            // Read ADC value
            adcValue = ADC1_ConversionResultGet();
            
            // Send log with ADC value
            sprintf(logBuffer, "ADC Value: %d\r\n", adcValue);
            USB_SendLog(logBuffer);
            
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

/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    cdc.h

  Summary:
    This header file provides prototypes and definitions for the CDC application.

  Description:
    This header file provides function prototypes and data type definitions for
    the CDC application.  Some of these are required by the system (such as the
    "CDC_Initialize" and "CDC_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "CDC_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _CDC_H
#define _CDC_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "usb/usb_device_cdc.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    CDC_STATE_INIT=0,
    CDC_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} CDC_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    CDC_STATES state;

    /* Device layer handle returned by USB_DEVICE_Open() */
    USB_DEVICE_HANDLE deviceHandle;

    /* Is device configured? */
    bool isConfigured;

    /* Set Line Coding Data */
    USB_CDC_LINE_CODING setLineCodingData;

    /* Get Line Coding Data */
    USB_CDC_LINE_CODING getLineCodingData;

    /* Control Line State */
    USB_CDC_CONTROL_LINE_STATE controlLineStateData;

    /* Send Break duration */
    uint16_t breakData;

    /* CDC instance number */
    USB_DEVICE_CDC_INDEX cdcInstance;

    /* Read transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE readTransferHandle;

    /* Write transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE writeTransferHandle;

    /* Read completion flag */
    bool cdcReadCompleted;

    /* Write completion flag */
    bool cdcWriteCompleted;

    /* Write Transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE wrTransferHandle;

    /* Read Transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE rdTransferHandle;

    /* Serial State Notification completed flag */
    bool cdcSerialStateNotificationCompleted;

    /* Button pressed flag */
    bool btnPressed;

} CDC_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

USB_DEVICE_CDC_EVENT_RESPONSE USBDeviceCDCEventHandler
(
    USB_DEVICE_CDC_INDEX instanceIndex,
    USB_DEVICE_CDC_EVENT event,
    void * pData,
    uintptr_t userData
);

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void CDC_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    CDC_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void CDC_Initialize ( void );


/*******************************************************************************
  Function:
    void CDC_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    CDC_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void CDC_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _CDC_H */

/*******************************************************************************
 End of File
 */

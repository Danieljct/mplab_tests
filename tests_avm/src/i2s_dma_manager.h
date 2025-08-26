/*******************************************************************************
  I2S DMA Manager Header File

  Company:
    Microchip Technology Inc.

  File Name:
    i2s_dma_manager.h

  Summary:
    I2S DMA Manager Interface Header File

  Description:
    This file provides the interface for managing I2S DMA ping-pong transfers
    using Harmony v3 framework. Based on the working Microchip Studio implementation.
*******************************************************************************/

#ifndef I2S_DMA_MANAGER_H
#define I2S_DMA_MANAGER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "definitions.h"
#include "audio/peripheral/i2s/plib_i2s.h"
#include "audio/driver/i2s/drv_i2s.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types and Constants
// *****************************************************************************
// *****************************************************************************

#define I2S_DMA_BUFFER_SIZE     12000   // 12000 samples per buffer

// I2S DMA Events
typedef enum
{
    I2S_DMA_EVENT_BUFFER_COMPLETE = 0,
    I2S_DMA_EVENT_ERROR,
    I2S_DMA_EVENT_NONE
} I2S_DMA_EVENT;

// Buffer status
typedef enum
{
    I2S_DMA_BUFFER_0 = 0,
    I2S_DMA_BUFFER_1
} I2S_DMA_BUFFER_ID;

// I2S DMA callback function pointer
typedef void (*I2S_DMA_CALLBACK)(I2S_DMA_EVENT event, I2S_DMA_BUFFER_ID bufferID, uint32_t* buffer, uintptr_t context);

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    bool I2S_DMA_Initialize(void)

  Summary:
    Initializes the I2S DMA ping-pong system

  Description:
    This function initializes the I2S peripheral and sets up the DMA ping-pong
    buffer system. It must be called before any other I2S_DMA functions.

  Precondition:
    System must be initialized and codec must be ready.

  Parameters:
    None

  Returns:
    true - Initialization successful
    false - Initialization failed

  Example:
    <code>
    if (I2S_DMA_Initialize())
    {
        // I2S DMA system ready
    }
    </code>

  Remarks:
    None.
*******************************************************************************/
bool I2S_DMA_Initialize(void);

/*******************************************************************************
  Function:
    bool I2S_DMA_Start(void)

  Summary:
    Starts the I2S DMA ping-pong transfers

  Description:
    This function starts the automatic ping-pong DMA transfers between the
    I2S peripheral and the two buffers. Once started, transfers will continue
    automatically until stopped.

  Precondition:
    I2S_DMA_Initialize() must have been called successfully.

  Parameters:
    None

  Returns:
    true - DMA transfers started successfully
    false - Failed to start transfers

  Example:
    <code>
    if (I2S_DMA_Start())
    {
        // Ping-pong transfers active
    }
    </code>

  Remarks:
    None.
*******************************************************************************/
bool I2S_DMA_Start(void);

/*******************************************************************************
  Function:
    void I2S_DMA_Stop(void)

  Summary:
    Stops the I2S DMA transfers

  Description:
    This function stops the DMA transfers and disables the DMA channel.

  Precondition:
    I2S_DMA_Start() must have been called.

  Parameters:
    None

  Returns:
    None

  Example:
    <code>
    I2S_DMA_Stop();
    </code>

  Remarks:
    None.
*******************************************************************************/
void I2S_DMA_Stop(void);

/*******************************************************************************
  Function:
    void I2S_DMA_CallbackRegister(I2S_DMA_CALLBACK callback, uintptr_t context)

  Summary:
    Registers a callback function for buffer complete events

  Description:
    This function registers a callback function that will be called whenever
    a buffer is completed. The callback provides the buffer ID and pointer
    to the completed buffer data.

  Precondition:
    I2S_DMA_Initialize() must have been called.

  Parameters:
    callback - Pointer to callback function
    context - User context to pass to callback

  Returns:
    None

  Example:
    <code>
    void MyI2SCallback(I2S_DMA_EVENT event, I2S_DMA_BUFFER_ID bufferID, 
                       uint32_t* buffer, uintptr_t context)
    {
        if (event == I2S_DMA_EVENT_BUFFER_COMPLETE)
        {
            // Process completed buffer
        }
    }
    
    I2S_DMA_CallbackRegister(MyI2SCallback, (uintptr_t)&myAppData);
    </code>

  Remarks:
    The callback executes in interrupt context.
*******************************************************************************/
void I2S_DMA_CallbackRegister(I2S_DMA_CALLBACK callback, uintptr_t context);

/*******************************************************************************
  Function:
    bool I2S_DMA_IsActive(void)

  Summary:
    Returns the status of the DMA transfers

  Description:
    This function returns whether the DMA ping-pong transfers are currently
    active or not.

  Precondition:
    I2S_DMA_Initialize() must have been called.

  Parameters:
    None

  Returns:
    true - DMA transfers are active
    false - DMA transfers are not active

  Example:
    <code>
    if (I2S_DMA_IsActive())
    {
        // Transfers are running
    }
    </code>

  Remarks:
    None.
*******************************************************************************/
bool I2S_DMA_IsActive(void);

/*******************************************************************************
  Function:
    uint32_t* I2S_DMA_GetBuffer(I2S_DMA_BUFFER_ID bufferID)

  Summary:
    Returns pointer to the specified buffer

  Description:
    This function returns a pointer to the specified buffer for direct access.

  Precondition:
    I2S_DMA_Initialize() must have been called.

  Parameters:
    bufferID - Buffer to get pointer for

  Returns:
    Pointer to the buffer, or NULL if invalid buffer ID

  Example:
    <code>
    uint32_t* buffer0 = I2S_DMA_GetBuffer(I2S_DMA_BUFFER_0);
    </code>

  Remarks:
    Use with caution - ensure buffer is not currently being written by DMA.
*******************************************************************************/
uint32_t* I2S_DMA_GetBuffer(I2S_DMA_BUFFER_ID bufferID);

/*******************************************************************************
  Function:
    uint32_t I2S_DMA_GetTransferCount(void)

  Summary:
    Returns the total number of completed transfers

  Description:
    This function returns the total number of ping-pong transfers that have
    been completed since the system started.

  Precondition:
    I2S_DMA_Initialize() must have been called.

  Parameters:
    None

  Returns:
    Total number of completed transfers

  Example:
    <code>
    uint32_t transfers = I2S_DMA_GetTransferCount();
    </code>

  Remarks:
    None.
*******************************************************************************/
uint32_t I2S_DMA_GetTransferCount(void);

#endif /* I2S_DMA_MANAGER_H */

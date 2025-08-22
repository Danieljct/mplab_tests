/*******************************************************************************
  I2S DMA Manager Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    i2s_dma_manager.c

  Summary:
    I2S DMA Manager Implementation

  Description:
    This file contains the implementation of the I2S DMA ping-pong manager
    based on the working Microchip Studio implementation.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "i2s_dma_manager.h"
#include <string.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

// Ping-pong buffers - aligned for DMA
static uint32_t i2sDmaBuffer0[I2S_DMA_BUFFER_SIZE] __attribute__((aligned(16)));
static uint32_t i2sDmaBuffer1[I2S_DMA_BUFFER_SIZE] __attribute__((aligned(16)));

// DMA descriptors - aligned and static for ping-pong
static dmac_descriptor_registers_t pingPongDescriptor0 __attribute__((aligned(16)));
static dmac_descriptor_registers_t pingPongDescriptor1 __attribute__((aligned(16)));

// Manager state
typedef struct
{
    bool isInitialized;
    bool isActive;
    volatile uint8_t currentBuffer; // 0 or 1 - indicates which buffer is being filled
    volatile uint32_t transferCount;
    volatile bool bufferReady;
    I2S_DMA_CALLBACK callback;
    uintptr_t context;
    DRV_HANDLE i2sHandle;
} I2S_DMA_MANAGER_DATA;

static I2S_DMA_MANAGER_DATA i2sDmaManager = {0};

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    static void I2S_DMA_Callback_Internal(DMAC_TRANSFER_EVENT event, uintptr_t context)

  Summary:
    Internal DMA callback function

  Description:
    This function handles DMA transfer complete and error events.
    It's called from the DMA interrupt context.
*******************************************************************************/
static void I2S_DMA_Callback_Internal(DMAC_TRANSFER_EVENT event, uintptr_t context)
{
    switch(event)
    {
        case DMAC_TRANSFER_EVENT_COMPLETE:
            // Buffer completed - similar to DMA_saveInSD_isr in Microchip Studio
            i2sDmaManager.bufferReady = true;
            i2sDmaManager.transferCount++;
            
            // Get the buffer that just completed
            uint8_t completedBufferID = i2sDmaManager.currentBuffer;
            uint32_t* completedBuffer = (completedBufferID == 0) ? i2sDmaBuffer0 : i2sDmaBuffer1;
            
            // Alternate to next buffer
            i2sDmaManager.currentBuffer = (i2sDmaManager.currentBuffer == 0) ? 1 : 0;
            
            // Call user callback if registered
            if (i2sDmaManager.callback != NULL)
            {
                i2sDmaManager.callback(I2S_DMA_EVENT_BUFFER_COMPLETE, 
                                     (I2S_DMA_BUFFER_ID)completedBufferID,
                                     completedBuffer, 
                                     i2sDmaManager.context);
            }
            break;
            
        case DMAC_TRANSFER_EVENT_ERROR:
            // DMA error occurred
            i2sDmaManager.bufferReady = true;
            
            if (i2sDmaManager.callback != NULL)
            {
                i2sDmaManager.callback(I2S_DMA_EVENT_ERROR, 
                                     I2S_DMA_BUFFER_0,  // Dummy buffer ID for error
                                     NULL, 
                                     i2sDmaManager.context);
            }
            break;
            
        default:
            break;
    }
}

/*******************************************************************************
  Function:
    static bool I2S_DMA_SetupPingPong(void)

  Summary:
    Sets up the ping-pong DMA descriptors

  Description:
    This function configures the DMA descriptors for ping-pong operation
    similar to the Microchip Studio implementation.
*******************************************************************************/
static bool I2S_DMA_SetupPingPong(void)
{
    // Disable DMA channel before configuration
    DMAC_ChannelDisable(DMAC_CHANNEL_0);
    
    // Configure descriptor 0 for buffer 0 -> points to descriptor 1
    DMAC_LinkedListDescriptorSetup(
        &pingPongDescriptor0,
        DMAC_BTCTRL_BLOCKACT_INT | DMAC_BTCTRL_BEATSIZE_WORD | 
        DMAC_BTCTRL_VALID_Msk | DMAC_BTCTRL_DSTINC_Msk,
        (const void*)&I2S_REGS->I2S_RXDATA,
        (const void*)i2sDmaBuffer0,
        sizeof(i2sDmaBuffer0),
        &pingPongDescriptor1
    );
    
    // Configure descriptor 1 for buffer 1 -> loop back to descriptor 0
    DMAC_LinkedListDescriptorSetup(
        &pingPongDescriptor1,
        DMAC_BTCTRL_BLOCKACT_INT | DMAC_BTCTRL_BEATSIZE_WORD | 
        DMAC_BTCTRL_VALID_Msk | DMAC_BTCTRL_DSTINC_Msk,
        (const void*)&I2S_REGS->I2S_RXDATA,
        (const void*)i2sDmaBuffer1,
        sizeof(i2sDmaBuffer1),
        &pingPongDescriptor0  // Loop back for continuous ping-pong
    );
    
    return true;
}

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions Implementation
// *****************************************************************************
// *****************************************************************************

bool I2S_DMA_Initialize(void)
{
    // Check if already initialized
    if (i2sDmaManager.isInitialized)
    {
        return true;
    }
    
    // Initialize manager state
    memset(&i2sDmaManager, 0, sizeof(I2S_DMA_MANAGER_DATA));
    
    // Initialize I2S PLIB
    I2S_Initialize();
    
    // Open I2S driver (required for Harmony compatibility)
    i2sDmaManager.i2sHandle = DRV_I2S_Open(0, DRV_IO_INTENT_READ);
    if (i2sDmaManager.i2sHandle == DRV_HANDLE_INVALID)
    {
        return false;
    }
    
    // Initialize buffers with debug patterns
    for(int i = 0; i < I2S_DMA_BUFFER_SIZE; i++) 
    {
        i2sDmaBuffer0[i] = 0xBEEF0000 + i;  // Buffer 0 pattern
        i2sDmaBuffer1[i] = 0xCAFE0000 + i;  // Buffer 1 pattern
    }
    
    // Initialize descriptors
    memset(&pingPongDescriptor0, 0, sizeof(dmac_descriptor_registers_t));
    memset(&pingPongDescriptor1, 0, sizeof(dmac_descriptor_registers_t));
    
    // Register DMA callback
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, I2S_DMA_Callback_Internal, 0);
    
    // Setup ping-pong descriptors
    if (!I2S_DMA_SetupPingPong())
    {
        return false;
    }
    
    i2sDmaManager.isInitialized = true;
    i2sDmaManager.currentBuffer = 0;  // Start with buffer 0
    
    return true;
}

bool I2S_DMA_Start(void)
{
    if (!i2sDmaManager.isInitialized)
    {
        return false;
    }
    
    if (i2sDmaManager.isActive)
    {
        return true;  // Already active
    }
    
    // Start linked list transfer with ping-pong descriptors
    if (DMAC_ChannelLinkedListTransfer(DMAC_CHANNEL_0, &pingPongDescriptor0))
    {
        i2sDmaManager.isActive = true;
        i2sDmaManager.transferCount = 0;
        return true;
    }
    
    return false;
}

void I2S_DMA_Stop(void)
{
    if (i2sDmaManager.isActive)
    {
        DMAC_ChannelDisable(DMAC_CHANNEL_0);
        i2sDmaManager.isActive = false;
    }
}

void I2S_DMA_CallbackRegister(I2S_DMA_CALLBACK callback, uintptr_t context)
{
    i2sDmaManager.callback = callback;
    i2sDmaManager.context = context;
}

bool I2S_DMA_IsActive(void)
{
    return i2sDmaManager.isActive;
}

uint32_t* I2S_DMA_GetBuffer(I2S_DMA_BUFFER_ID bufferID)
{
    if (!i2sDmaManager.isInitialized)
    {
        return NULL;
    }
    
    switch(bufferID)
    {
        case I2S_DMA_BUFFER_0:
            return i2sDmaBuffer0;
        case I2S_DMA_BUFFER_1:
            return i2sDmaBuffer1;
        default:
            return NULL;
    }
}

uint32_t I2S_DMA_GetTransferCount(void)
{
    return i2sDmaManager.transferCount;
}

/*******************************************************************************
  Function:
    bool I2S_DMA_ProcessCompletedBuffer(void)

  Summary:
    Processes any completed buffers (non-blocking)

  Description:
    This function should be called from the main loop to process completed
    buffers. It returns true if a buffer was processed.
*******************************************************************************/
bool I2S_DMA_ProcessCompletedBuffer(void)
{
    if (i2sDmaManager.bufferReady)
    {
        i2sDmaManager.bufferReady = false;
        return true;
    }
    return false;
}

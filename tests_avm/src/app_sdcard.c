/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_sdcard.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_sdcard.h"
#include "i2s_dma_manager.h"
#include "peripheral/rtc/plib_rtc.h"
#include "system/console/sys_console.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define SDCARD_MOUNT_NAME    SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0
#define SDCARD_DEV_NAME      SYS_FS_MEDIA_IDX0_DEVICE_NAME_VOLUME_IDX0
#define SDCARD_FILE_PREFIX   "audio_data"

#define BUILD_TIME_HOUR     ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define BUILD_TIME_MIN      ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define BUILD_TIME_SEC      ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')

#define LOG_TIME_LEN        10
#define LOG_BUFFER_LEN      64
#define LOG_LEN             (LOG_TIME_LEN + LOG_BUFFER_LEN)

// WAV header parameters
#define WAV_NUM_CHANNELS    2
#define WAV_SAMPLE_RATE     12000
#define WAV_BITS_PER_SAMPLE 16
#define WAV_BLOCK_ALIGN     (WAV_NUM_CHANNELS * WAV_BITS_PER_SAMPLE / 8)
#define WAV_BYTE_RATE       (WAV_SAMPLE_RATE * WAV_BLOCK_ALIGN)
#define WAV_HEADER_SIZE     44

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_SDCARD_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_SDCARD_DATA appSDCARDData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void APP_SDCARD_DMABufferReady(uint8_t bufferID, uint32_t* buffer, uint32_t bufferSize)
{
    /* DMA Buffer ready. Store buffer information for writing to SD */
    appSDCARDData.dmaBufferID = bufferID;
    appSDCARDData.dmaBuffer = buffer;
    appSDCARDData.dmaBufferSize = bufferSize;
    appSDCARDData.isDmaBufferReady = true;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
static void APP_SysFSEventHandler(SYS_FS_EVENT event,void* eventData,uintptr_t context)
{
    switch(event)
    {
        /* If the event is mount then check which media has been mounted */
        case SYS_FS_EVENT_MOUNT:
            if(strcmp((const char *)eventData, SDCARD_MOUNT_NAME) == 0)
            {
                /* Set SDCARD Mount flag. Here -----> Step #3 */
                appSDCARDData.sdCardMountFlag = true;
            }
            break;

        /* If the event is unmount then check which media has been unmount */
        case SYS_FS_EVENT_UNMOUNT:
            if(strcmp((const char *)eventData, SDCARD_MOUNT_NAME) == 0)
            {
                appSDCARDData.sdCardMountFlag = false;
            }

            if (appSDCARDData.state != APP_SDCARD_STATE_IDLE)
            {
                SYS_CONSOLE_PRINT("!!! WARNING SDCARD Ejected Abruptly !!!\r\n\r\n");

                LED_G_Clear();

                appSDCARDData.state = APP_SDCARD_STATE_ERROR;
            }
            break;

        case SYS_FS_EVENT_ERROR:
        default:
            break;
    }
}

static void WriteWavHeader(SYS_FS_HANDLE fileHandle, uint32_t dataSize)
{
    uint8_t header[WAV_HEADER_SIZE] = {
        // RIFF chunk descriptor
        'R','I','F','F',
        0,0,0,0, // ChunkSize (to be filled)
        'W','A','V','E',
        // fmt subchunk
        'f','m','t',' ',
        16,0,0,0, // Subchunk1Size (16 for PCM)
        1,0, // AudioFormat (1 = PCM)
        WAV_NUM_CHANNELS,0, // NumChannels
        WAV_SAMPLE_RATE & 0xFF, (WAV_SAMPLE_RATE >> 8) & 0xFF, (WAV_SAMPLE_RATE >> 16) & 0xFF, (WAV_SAMPLE_RATE >> 24) & 0xFF, // SampleRate
        WAV_BYTE_RATE & 0xFF, (WAV_BYTE_RATE >> 8) & 0xFF, (WAV_BYTE_RATE >> 16) & 0xFF, (WAV_BYTE_RATE >> 24) & 0xFF, // ByteRate
        WAV_BLOCK_ALIGN,0, // BlockAlign
        WAV_BITS_PER_SAMPLE,0, // BitsPerSample
        // data subchunk
        'd','a','t','a',
        0,0,0,0 // Subchunk2Size (to be filled)
    };

    uint32_t chunkSize = dataSize + 36;
    uint32_t subchunk2Size = dataSize;

    // Set ChunkSize
    header[4] = (chunkSize) & 0xFF;
    header[5] = (chunkSize >> 8) & 0xFF;
    header[6] = (chunkSize >> 16) & 0xFF;
    header[7] = (chunkSize >> 24) & 0xFF;

    // Set Subchunk2Size
    header[40] = (subchunk2Size) & 0xFF;
    header[41] = (subchunk2Size >> 8) & 0xFF;
    header[42] = (subchunk2Size >> 16) & 0xFF;
    header[43] = (subchunk2Size >> 24) & 0xFF;

    SYS_FS_FileSeek(fileHandle, 0, SYS_FS_SEEK_SET);
    SYS_FS_FileWrite(fileHandle, header, WAV_HEADER_SIZE);
}

static void UpdateWavHeader(SYS_FS_HANDLE fileHandle, uint32_t dataSize)
{
    WriteWavHeader(fileHandle, dataSize);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_SDCARD_Initialize ( void )

  Remarks:
    See prototype in app_sdcard.h.
 */

void APP_SDCARD_Initialize ( void )
{
    struct tm sys_time;

    /* Intialize the app state to wait for media attach. */
    appSDCARDData.state                     = APP_SDCARD_STATE_MOUNT_WAIT;

    appSDCARDData.isDmaBufferReady          = false;
    appSDCARDData.dmaBufferID               = 0;
    appSDCARDData.dmaBuffer                 = NULL;
    appSDCARDData.dmaBufferSize             = 0;
   
    appSDCARDData.sdCardMountFlag           = false;

    sys_time.tm_hour                        = BUILD_TIME_HOUR;
    sys_time.tm_min                         = BUILD_TIME_MIN;
    sys_time.tm_sec                         = BUILD_TIME_SEC;

    /* Set RTC Time to current system time. */
    RTC_RTCCTimeSet(&sys_time);

    /* Register the File System Event handler. */
    SYS_FS_EventHandlerSet(APP_SysFSEventHandler,(uintptr_t)NULL);
}

/******************************************************************************
  Function:
    void APP_SDCARD_Tasks ( void )

  Remarks:
    See prototype in app_sdcard.h.
 */
void APP_SDCARD_Tasks ( void )
{
    switch (appSDCARDData.state)
    {
        case APP_SDCARD_STATE_MOUNT_WAIT:
        {
            /* Wait for SDCARD to be Auto Mounted. Here -----> Step #4 */
            if(appSDCARDData.sdCardMountFlag == true)
            {
                appSDCARDData.state = APP_SDCARD_STATE_OPEN_FILE;
                appSDCARDData.sdCardMountFlag = false;
            }
            break;
        }

        case APP_SDCARD_STATE_OPEN_FILE:
        {
            /* Open Audio Data Log file with timestamp. */
            static char fileName[64];
            struct tm sys_time;
            RTC_RTCCTimeGet(&sys_time);
            
            snprintf(fileName, sizeof(fileName), SDCARD_MOUNT_NAME"/"SDCARD_FILE_PREFIX"_%02d%02d%02d.wav",
                    sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec);

            appSDCARDData.fileHandle = SYS_FS_FileOpen(fileName, (SYS_FS_FILE_OPEN_WRITE));

            if(appSDCARDData.fileHandle == SYS_FS_HANDLE_INVALID)
            {
                /* Could not open the file. Error out*/
                appSDCARDData.state = APP_SDCARD_STATE_ERROR;
                break;
            }

            // Escribe el encabezado WAV inicial (sin datos)
            WriteWavHeader(appSDCARDData.fileHandle, 0);
            appSDCARDData.totalBytesWritten = 0;

            SYS_CONSOLE_PRINT("Audio file opened: %s\r\n", fileName);
            appSDCARDData.state = APP_SDCARD_STATE_WRITE;

            break;
        }

        case APP_SDCARD_STATE_WRITE:
        {
            /* Check if DMA buffer data is ready to be written to SDCARD. */
            if (appSDCARDData.isDmaBufferReady == true)
            {
                struct tm sys_time;
                static uint32_t bufferCount = 0;
                
                SYS_CONSOLE_PRINT("Writing DMA buffer %d to SDCARD...", appSDCARDData.dmaBufferID);

                appSDCARDData.isDmaBufferReady = false;

                /* Get System Time from RTC. */
                RTC_RTCCTimeGet(&sys_time);

                // Escribe los datos después del encabezado WAV
                SYS_FS_FileSeek(appSDCARDData.fileHandle, WAV_HEADER_SIZE + appSDCARDData.totalBytesWritten, SYS_FS_SEEK_SET);
                size_t bytesWritten = SYS_FS_FileWrite(appSDCARDData.fileHandle, 
                                                      appSDCARDData.dmaBuffer, 
                                                      appSDCARDData.dmaBufferSize);

                if(bytesWritten != appSDCARDData.dmaBufferSize)
                {
                    /* There was an error while writing the file */
                    SYS_CONSOLE_PRINT("Error writing to file!\r\n");
                    appSDCARDData.state = APP_SDCARD_STATE_ERROR;
                }
                else
                {
                    /* The write was successful */
                    bufferCount++;
                    appSDCARDData.totalBytesWritten += bytesWritten;
                    SYS_CONSOLE_PRINT("Done! Buffer #%u written (%u bytes) at [%02d:%02d:%02d]\r\n", 
                                     bufferCount, bytesWritten, 
                                     sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec);

                    LED_G_Toggle();

                    /* Force flush to ensure data is written to SD card */
                    SYS_FS_FileSync(appSDCARDData.fileHandle);

                    /* Continue waiting for more DMA buffers */
                    appSDCARDData.state = APP_SDCARD_STATE_WRITE;
                }
            }
            break;
        }

        case APP_SDCARD_STATE_CLOSE_FILE:
        {
            // Actualiza el encabezado WAV con el tamaño final de los datos
            UpdateWavHeader(appSDCARDData.fileHandle, appSDCARDData.totalBytesWritten);

            SYS_FS_FileClose(appSDCARDData.fileHandle);

            SYS_CONSOLE_PRINT("Audio logging to SDCARD Stopped \r\n");
            SYS_CONSOLE_PRINT("Safe to Eject SDCARD \r\n\r\n");

            LED_G_Clear();

            appSDCARDData.state = APP_SDCARD_STATE_IDLE;

            break;
        }

        case APP_SDCARD_STATE_ERROR:
        {
            SYS_CONSOLE_PRINT("SDCARD Task Error \r\n\r\n");
            appSDCARDData.state = APP_SDCARD_STATE_IDLE;
            break;
        }

        case APP_SDCARD_STATE_IDLE:
        default:
        {
            break;
        }
    }
}

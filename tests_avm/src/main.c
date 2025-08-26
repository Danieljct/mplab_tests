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

#include "cdc.h"
#include "app_usb.h"
#include "app_sdcard.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"
#include <stdio.h>
#include "codec.h"
#include "ble_slave.h"   
#include "sd_handler.h"
#include "i2s_dma_manager.h"  // Nuevo manager I2S/DMA

static bool volatile bToggleLED = false;
static uint16_t volatile adcValue = 0;
static bool codecInitDone = false;
static bool sdInitDone = false;
static bool i2sInitDone = false;

// Variables para SPI slave
static uint8_t spiRxBuffer[256];
static volatile bool spiCommandReceived = false;
static volatile uint8_t lastCommand = 0;
static volatile size_t lastBytesReceived = 0;

int* real_i2s;
// This function is called after period expires
void TC0_CH0_TimerInterruptHandler(uint32_t status, uintptr_t context)
{
    bToggleLED = true;
}

// Callback para TC1 - controla lectura I2S a 1kHz
void TC1_CH0_TimerInterruptHandler(uint32_t status, uintptr_t context)
{
    LED_B_Toggle();
}

// Callback para cuando se completa una transacción SPI
void SPI_SlaveCallback(uintptr_t context)
{
    // Verificar si hay errores primero
    SPI_SLAVE_ERROR error = SERCOM2_SPI_ErrorGet();
    if (error != SPI_SLAVE_ERROR_NONE)
    {
        return;
    }
    
    // Obtener cuántos bytes fueron recibidos
    lastBytesReceived = SERCOM2_SPI_ReadCountGet();
    
    if (lastBytesReceived > 0)
    {
        // Leer los datos recibidos
        size_t bytesRead = SERCOM2_SPI_Read(spiRxBuffer, sizeof(spiRxBuffer));
        
        if (bytesRead > 0 && spiRxBuffer[0] != 0x00)
        {
            lastCommand = spiRxBuffer[0];
            spiCommandReceived = true;
        }
    }
}

// Función para procesar comandos SPI recibidos
void ProcessSPICommands(void)
{
    if (spiCommandReceived)
    {
        // Usar el nuevo sistema BLE para procesar comandos
        BLE_processCommand(lastCommand, spiRxBuffer, lastBytesReceived);
        
        // Resetear flag
        spiCommandReceived = false;
    }
}

// Callback para cuando se completa un buffer I2S DMA
void I2S_BufferCompleteCallback(I2S_DMA_EVENT event, I2S_DMA_BUFFER_ID bufferID, 
                                uint32_t* buffer, uintptr_t context)
{
    static uint32_t transferCount = 0;
    
    if (event == I2S_DMA_EVENT_BUFFER_COMPLETE)
    {
        transferCount++;
        
        SYS_CONSOLE_PRINT("=== BUFFER %d COMPLETO (Transfer #%u) ===\r\n", 
                          bufferID, transferCount);
        
        // Mostrar algunas muestras
        if (buffer != NULL)
        {
            SYS_CONSOLE_PRINT("Primeras 4 muestras: 0x%08X 0x%08X 0x%08X 0x%08X\r\n",
                              buffer[0], buffer[1], buffer[2], buffer[3]);
            
            // Verificar si contiene datos reales del I2S
            bool hasI2SData = false;
            if (bufferID == I2S_DMA_BUFFER_0)
            {
                hasI2SData = (buffer[0] != 0xBEEF0000) || (buffer[1] != 0xBEEF0001);
            }
            else
            {
                hasI2SData = (buffer[0] != 0xCAFE0000) || (buffer[1] != 0xCAFE0001);
            }
            
            // Notificar al sistema SD Card que hay un buffer DMA listo
            APP_SDCARD_DMABufferReady((uint8_t)bufferID, buffer, I2S_DMA_BUFFER_SIZE * 4);
        }
    }
    else if (event == I2S_DMA_EVENT_ERROR)
    {
        SYS_CONSOLE_PRINT("ERROR en I2S DMA!\r\n");
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****

int main ( void )
{
    // Initialize all modules
    SYS_Initialize(NULL);
    SYSTICK_TimerStart();

    // Initialize SD Card application
    APP_SDCARD_Initialize();

    // Initialize SPI Slave
    SERCOM2_SPI_Initialize();
    SERCOM2_SPI_CallbackRegister(SPI_SlaveCallback, 0);
    
    // Initialize BLE slave system
    BLE_slave_init();
    
    // Initialize power pins
    PWR_LDOON_Set();
    PWR_BON_Set();
    PWR_HPWR_Set();
    PWR_SUSP_Clear();
    PWR_STBY_Clear();
    PWR_SD_LDO_EN_Set();
    COD_RESET_Set();
    
    // Enable ADC1
    ADC1_Enable();

    // Register callback function for CH0 period interrupt
    TC0_TimerCallbackRegister(TC0_CH0_TimerInterruptHandler, (uintptr_t)NULL);

    // Register callback function for TC1 CH0 period interrupt (1kHz para I2S)
    TC1_TimerCallbackRegister(TC1_CH0_TimerInterruptHandler, (uintptr_t)NULL);

    // Start the timer channel 0
    TC0_TimerStart();
    
    // Start TC1 timer for I2S sampling
    TC1_TimerStart();
    
    SYS_CONSOLE_PRINT("=== Sistema AVM Iniciado ===\r\n");
    
    while ( true )
    {
        // Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks();
        
        // Handle SD Card tasks
        APP_SDCARD_Tasks();
        
        // Initialize codec once after system stabilization
        if (!codecInitDone)
        {
            static uint32_t initCounter = 0;
            initCounter++;
            
            if (initCounter > 5000)
            {
                CodecGain_t codecGain;
                codecGain.leftGain = 0x50;
                codecGain.rightGain = 0x50;
                codecGain.micGain = 0x40;
                
                CODEC_init(codecGain);
                codecInitDone = true;
                CODEC_printAllRegisters();
                
                SYS_CONSOLE_PRINT("Codec inicializado\r\n");
            }
        }
        
        // Initialize I2S DMA system after codec is ready
        if (!i2sInitDone && codecInitDone)
        {
            static uint32_t i2sInitCounter = 0;
            i2sInitCounter++;
            
            if (i2sInitCounter > 1000)
            {
                // Inicializar I2S DMA Manager
                if (I2S_DMA_Initialize())
                {
                    // Registrar callback para buffers completos
                    I2S_DMA_CallbackRegister(I2S_BufferCompleteCallback, 0);
                    
                    // Iniciar ping-pong transfers
                    if (I2S_DMA_Start())
                    {
                        i2sInitDone = true;
                        SYS_CONSOLE_PRINT("I2S DMA Ping-Pong iniciado correctamente!\r\n");
                        SYS_CONSOLE_PRINT("Transfers automáticos activos - Buffer size: %d samples\r\n", 
                                          I2S_DMA_BUFFER_SIZE);
                    }
                    else
                    {
                        SYS_CONSOLE_PRINT("Error al iniciar I2S DMA transfers\r\n");
                    }
                }
                else
                {
                    SYS_CONSOLE_PRINT("Error al inicializar I2S DMA Manager\r\n");
                }
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
            bToggleLED = false;
            LED_R_Toggle();
            
            // Escribir datos periódicos a SD si está lista
            static uint32_t dataCounter = 0;
            if(SD_IsReady() && (dataCounter % 1000 == 0))
            {
                char dataFile[64];
                char sensorData[256];
                
                snprintf(dataFile, sizeof(dataFile), "sensor_%u.txt", dataCounter / 1000);
                snprintf(sensorData, sizeof(sensorData), 
                    "Timestamp: %u\r\nADC Value: %d\r\nCounter: %u\r\nI2S Transfers: %u\r\n", 
                    SYSTICK_GetTickCounter(), adcValue, dataCounter, I2S_DMA_GetTransferCount());
                
                SD_WriteFile(dataFile, sensorData, strlen(sensorData));
                SYS_CONSOLE_PRINT("Datos de sensor escritos: %s\r\n", dataFile);
            }
            dataCounter++;
        }
    }

    return EXIT_FAILURE;
}
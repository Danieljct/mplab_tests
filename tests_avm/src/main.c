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
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <stdio.h>
#include "codec.h"                      // Codec driver
#include "ble_slave.h"   
#include "sd_handler.h"                 // SD Card handler
#include "audio/peripheral/i2s/plib_i2s.h"  // I2S driver
#include "audio/driver/i2s/drv_i2s.h"   // I2S driver

static bool volatile bToggleLED = false;
static uint16_t volatile adcValue = 0;
static bool codecInitDone = false;
static bool sdInitDone = false;
static bool i2sInitDone = false;
static DRV_HANDLE i2sHandle = DRV_HANDLE_INVALID;

// Variables para SPI slave
static uint8_t spiRxBuffer[256];
static volatile bool spiCommandReceived = false;
static volatile uint8_t lastCommand = 0;
static volatile size_t lastBytesReceived = 0;

// This function is called after period expires
void TC0_CH0_TimerInterruptHandler(uint32_t status, uintptr_t context)
{
    bToggleLED = true;
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
    
    // Initialize SPI Slave
    SERCOM2_SPI_Initialize();
    SERCOM2_SPI_CallbackRegister(SPI_SlaveCallback, 0);
    
    // Initialize BLE slave system
    BLE_slave_init();
    
    // Initialize I2S PLIB first
    I2S_Initialize();
    
    // Wait a bit for I2S to stabilize
    for(volatile uint32_t i = 0; i < 10000; i++);
    
    // Open I2S driver handle
    i2sHandle = DRV_I2S_Open(0, DRV_IO_INTENT_READ);
    if(i2sHandle != DRV_HANDLE_INVALID)
    {
        i2sInitDone = true;
        SYS_CONSOLE_PRINT("I2S Driver inicializado correctamente\r\n");
    }
    else
    {
        SYS_CONSOLE_PRINT("Error al abrir I2S Driver\r\n");
    }
    
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

    // Start the timer channel 0
    TC0_TimerStart();
    
    SYS_CONSOLE_PRINT("=== Sistema AVM Iniciado ===\r\n");
    
    while ( true )
    {
        // Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks();
        
        // Procesar comandos SPI recibidos
       // ProcessSPICommands();
        
        // Leer datos I2S usando buffer queue (sin callback para simplificar)
        if (i2sInitDone && i2sHandle != DRV_HANDLE_INVALID)
        {
            static uint32_t i2sReadCounter = 0;
            static uint32_t i2sBuffer[64]; // Buffer para leer datos I2S
            static DRV_I2S_BUFFER_HANDLE bufferHandle = DRV_I2S_BUFFER_HANDLE_INVALID;
            static bool readInProgress = false;
            
            i2sReadCounter++;
            
            // Si no hay lectura en progreso, iniciar una nueva
            if (!readInProgress && (i2sReadCounter % 100000 == 0))
            {
                DRV_I2S_ReadBufferAdd(i2sHandle, i2sBuffer, sizeof(i2sBuffer), &bufferHandle);
                
                if (bufferHandle != DRV_I2S_BUFFER_HANDLE_INVALID)
                {
                    readInProgress = true;
                    SYS_CONSOLE_PRINT("I2S Read iniciado [%u]\r\n", i2sReadCounter / 100000);
                }
                else
                {
                    SYS_CONSOLE_PRINT("Error al iniciar I2S Read\r\n");
                }
            }
            
            // Verificar si la lectura se completó
            if (readInProgress && bufferHandle != DRV_I2S_BUFFER_HANDLE_INVALID)
            {
                DRV_I2S_BUFFER_EVENT status = DRV_I2S_BufferStatusGet(bufferHandle);
                
                if (status == DRV_I2S_BUFFER_EVENT_COMPLETE)
                {
                    // Imprimir algunos datos del buffer
                    SYS_CONSOLE_PRINT("I2S Read OK: 0x%08X 0x%08X 0x%08X 0x%08X\r\n", 
                        i2sBuffer[0], i2sBuffer[1], i2sBuffer[2], i2sBuffer[3]);
                    
                    readInProgress = false;
                    bufferHandle = DRV_I2S_BUFFER_HANDLE_INVALID;
                }
                else if (status == DRV_I2S_BUFFER_EVENT_ERROR)
                {
                    SYS_CONSOLE_PRINT("I2S Read Error\r\n");
                    readInProgress = false;
                    bufferHandle = DRV_I2S_BUFFER_HANDLE_INVALID;
                }
                // Si está en progreso, seguir esperando
            }
        }
        
        // Initialize codec once after system stabilization
        if (!codecInitDone)
        {
            static uint32_t initCounter = 0;
            initCounter++;
            
            if (initCounter > 5000) // Reducir tiempo de espera para codec
            {
                CodecGain_t codecGain;
                codecGain.leftGain = 0x50;   // Default gain value
                codecGain.rightGain = 0x50;  // Default gain value  
                codecGain.micGain = 0x40;    // Default mic gain
                
                CODEC_init(codecGain);
                codecInitDone = true;
                
                SYS_CONSOLE_PRINT("Codec inicializado\r\n");
            }
        }
        /*
        // Initialize SD Card system - darle más tiempo después del codec
        if (!sdInitDone && codecInitDone)
        {
            static uint32_t sdInitCounter = 0;
            sdInitCounter++;
            
            if (sdInitCounter > 3000) // Menos tiempo de espera ya que el codec está listo
            {
                SYS_CONSOLE_PRINT("Iniciando sistema SD...\r\n");
                
                SD_Result_t result = SD_Initialize();
                if(result == SD_SUCCESS)
                {
                    SYS_CONSOLE_PRINT("SD inicializada correctamente\r\n");
                    
                    // Mostrar información de la SD
                    SD_PrintInfo();
                    
                    // Listar archivos existentes
                    SD_ListFiles();
                    
                    // Escribir archivo de prueba
                    SD_WriteTestFile();
                    
                    // Listar archivos nuevamente para ver el archivo creado
                    SD_ListFiles();
                }
                else
                {
                    SYS_CONSOLE_PRINT("Error inicializando SD: %d\r\n", result);
                    SYS_CONSOLE_PRINT("Reintentando en próximo ciclo...\r\n");
                }
                
                sdInitDone = true;
            }
        }
         */

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
            if(SD_IsReady() && (dataCounter % 10000 == 0)) // Cada ~10 segundos aprox
            {
                char dataFile[64];
                char sensorData[256];
                
                snprintf(dataFile, sizeof(dataFile), "sensor_%u.txt", dataCounter / 10000);
                snprintf(sensorData, sizeof(sensorData), 
                    "Timestamp: %u\r\nADC Value: %d\r\nCounter: %u\r\n", 
                    SYSTICK_GetTickCounter(), adcValue, dataCounter);
                
                SD_WriteFile(dataFile, sensorData, strlen(sensorData));
                SYS_CONSOLE_PRINT("Datos de sensor escritos: %s\r\n", dataFile);
            }
            dataCounter++;
        }
    }

    // Execution should not come here during normal operation
    return EXIT_FAILURE;
}

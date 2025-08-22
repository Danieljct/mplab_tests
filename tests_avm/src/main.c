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
static bool volatile bReadI2S = false;  // Flag para lectura I2S controlada por timer

// Buffer para DMA I2S - 10000 datos de 32 bits
static uint32_t i2sDmaBuffer[10000] __attribute__((aligned(4)));
static volatile bool dmaTransferInProgress = false;
static volatile uint32_t dmaTransferCount = 0;
static volatile uint32_t lastTransferTime = 0;
static volatile uint32_t transferStartTime = 0;

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
    bReadI2S = true;
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

// Callback para DMA I2S
void I2S_DMA_Callback(DMAC_TRANSFER_EVENT event, uintptr_t context)
{
    switch(event)
    {
        case DMAC_TRANSFER_EVENT_COMPLETE:
            SYS_CONSOLE_PRINT("DMA I2S Transfer Complete - 10000 samples received!\r\n");
            dmaTransferInProgress = false;
            break;
            
        case DMAC_TRANSFER_EVENT_ERROR:
            SYS_CONSOLE_PRINT("DMA I2S Transfer Error!\r\n");
            dmaTransferInProgress = false;
            break;
            
        default:
            break;
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
    
    // Initialize I2S PLIB first (but don't open driver yet)
    I2S_Initialize();
    
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
    
    // Register DMA callback for channel 0
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, I2S_DMA_Callback, 0);
    
    SYS_CONSOLE_PRINT("=== Sistema AVM Iniciado ===\r\n");
    
    while ( true )
    {
        // Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks();
        
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
                
                SYS_CONSOLE_PRINT("Codec inicializado\r\n");
            }
        }
        
        // Initialize I2S driver after codec is ready
        if (!i2sInitDone && codecInitDone)
        {
            static uint32_t i2sInitCounter = 0;
            i2sInitCounter++;
            
            if (i2sInitCounter > 1000)
            {
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
            }
        }
        
        // DMA I2S Transfer cuando esté listo
        if (i2sInitDone && !dmaTransferInProgress && bReadI2S)
        {
            bReadI2S = false;
            
            // Limpiar buffer antes de transferencia
            for(int i = 0; i < 10000; i++) i2sDmaBuffer[i] = 0xDEADBEEF;
            
            // Iniciar transferencia DMA
            bool dmaResult = DMAC_ChannelTransfer(DMAC_CHANNEL_0, 
                                                (const void*)&I2S_REGS->I2S_RXDATA, 
                                                (const void*)i2sDmaBuffer, 
                                                40000); // 10000 * 4 bytes
            
            if (dmaResult)
            {
                dmaTransferInProgress = true;
                SYS_CONSOLE_PRINT("DMA I2S Transfer iniciado - esperando 10000 samples...\r\n");
            }
            else
            {
                SYS_CONSOLE_PRINT("Error al iniciar DMA I2S Transfer\r\n");
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
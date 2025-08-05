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

static bool volatile bToggleLED = false;
static uint16_t volatile adcValue = 0;
static bool codecTestDone = false;

// This function is called after period expires
void TC0_CH0_TimerInterruptHandler(uint32_t status, uintptr_t context)
{
    bToggleLED = true;
}

// Simple I2C scanner function using synchronous mode
void I2C_ScanDevices(void)
{
    DRV_HANDLE i2cHandle;
    DRV_I2C_TRANSFER_SETUP transferSetup;
    uint8_t testByte = 0x00;
    int devicesFound = 0;
    
    SYS_CONSOLE_PRINT("\r\n=== I2C Device Scanner (Synchronous Mode) ===\r\n");
    SYS_CONSOLE_PRINT("Scanning I2C bus for devices...\r\n");
    SYS_CONSOLE_PRINT("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    
    // Open I2C driver
    i2cHandle = DRV_I2C_Open(DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE);
    
    if (i2cHandle == DRV_HANDLE_INVALID)
    {
        SYS_CONSOLE_PRINT("Failed to open I2C driver!\r\n");
        return;
    }
    
    // Configure I2C for 100kHz
    transferSetup.clockSpeed = 100000;
    if (!DRV_I2C_TransferSetup(i2cHandle, &transferSetup))
    {
        SYS_CONSOLE_PRINT("Failed to configure I2C!\r\n");
        DRV_I2C_Close(i2cHandle);
        return;
    }
    CODEC_resetHw();
    // Scan addresses 0x08 to 0x77 (valid 7-bit I2C addresses)
    for (uint8_t addr = 0x08; addr <= 0x77; addr++)
    {
        if (addr % 16 == 0)
        {
            SYS_CONSOLE_PRINT("%02x: ", addr);
        }
        
        // Try to write to the device using synchronous mode (just address, no data)
        if (DRV_I2C_WriteTransfer(i2cHandle, addr, &testByte, 0))
        {
            SYS_CONSOLE_PRINT("%02x ", addr);
            devicesFound++;
            
            // Special note for codec address
            if (addr == CODEC_ADDRESS)
            {
                SYS_CONSOLE_PRINT("(CODEC) ");
            }
        }
        else
        {
            SYS_CONSOLE_PRINT("-- ");
        }
        
        if ((addr + 1) % 16 == 0)
        {
            SYS_CONSOLE_PRINT("\r\n");
        }
        
        SYSTICK_DelayMs(5); // Small delay between scans
    }
    
    SYS_CONSOLE_PRINT("\r\n");
    SYS_CONSOLE_PRINT("Scan complete. Found %d device(s).\r\n", devicesFound);
    
    if (devicesFound == 0)
    {
        SYS_CONSOLE_PRINT("No I2C devices found. Check connections and pull-up resistors.\r\n");
    }
    
    SYS_CONSOLE_PRINT("=== I2C Scanner Complete ===\r\n\r\n");
    
    DRV_I2C_Close(i2cHandle);
}

// Función mejorada para diagnóstico del codec
void CODEC_DiagnosticTest(void)
{
    CodecResult_t result;
    uint8_t regValue;
    uint8_t testValues[] = {0x00, 0x55, 0xAA, 0xFF, 0x33, 0xCC};
    int testCount = sizeof(testValues) / sizeof(testValues[0]);
    int successCount = 0;
    
    SYS_CONSOLE_PRINT("\r\n=== CODEC Diagnostic Test ===\r\n");
    
    // 1. Verificar si el codec responde (leer registro de ID si existe)
    SYS_CONSOLE_PRINT("1. Testing basic communication...\r\n");
    result = CODEC_readRegister(0x02, &regValue); // Registro 0 (suele ser ID o reset)
    if (result == CODEC_SUCCESS)
    {
        SYS_CONSOLE_PRINT("   Basic read: SUCCESS - Reg 0x00 = 0x%02X\r\n", regValue);
    }
    else
    {
        SYS_CONSOLE_PRINT("   Basic read: FAILED\r\n");
    }
    
    // 2. Probar múltiples registros de solo lectura para verificar comunicación
    SYS_CONSOLE_PRINT("2. Testing multiple register reads...\r\n");
    for (uint8_t reg = 0; reg < 8; reg++) // Probar primeros 8 registros
    {
        result = CODEC_readRegister(reg, &regValue);
        if (result == CODEC_SUCCESS)
        {
            SYS_CONSOLE_PRINT("   Reg 0x%02X = 0x%02X\r\n", reg, regValue);
            successCount++;
        }
        else
        {
            SYS_CONSOLE_PRINT("   Reg 0x%02X = READ FAILED\r\n", reg);
        }
        SYSTICK_DelayMs(10); // Pequeña pausa entre lecturas
    }
    
    // 3. Test de escritura/lectura en registro seguro (no crítico)
    SYS_CONSOLE_PRINT("3. Write/Read test with multiple values...\r\n");
    
    // Primero leer el valor original
    uint8_t originalValue;
    result = CODEC_readRegister(CODEC_REG_SAMPLE_RATE_SELECT, &originalValue);
    if (result == CODEC_SUCCESS)
    {
        SYS_CONSOLE_PRINT("   Original value: 0x%02X\r\n", originalValue);
    }
    
    // Probar diferentes valores
    int writeReadSuccess = 0;
    for (int i = 0; i < testCount; i++)
    {
        // Escribir valor de prueba
        result = CODEC_writeRegister(CODEC_REG_SAMPLE_RATE_SELECT, testValues[i]);
        if (result != CODEC_SUCCESS)
        {
            SYS_CONSOLE_PRINT("   Write 0x%02X: FAILED\r\n", testValues[i]);
            continue;
        }
        
        // Pequeña pausa después de escribir
        SYSTICK_DelayMs(5);
        
        // Leer el valor
        result = CODEC_readRegister(CODEC_REG_SAMPLE_RATE_SELECT, &regValue);
        if (result == CODEC_SUCCESS)
        {
            if (regValue == testValues[i])
            {
                SYS_CONSOLE_PRINT("   Write/Read 0x%02X: SUCCESS �??\r\n", testValues[i]);
                writeReadSuccess++;
            }
            else
            {
                SYS_CONSOLE_PRINT("   Write/Read 0x%02X: MISMATCH (got 0x%02X) �??\r\n", 
                                 testValues[i], regValue);
            }
        }
        else
        {
            SYS_CONSOLE_PRINT("   Write 0x%02X, Read: FAILED �??\r\n", testValues[i]);
        }
    }
    
    // Restaurar valor original
    CODEC_writeRegister(CODEC_REG_SAMPLE_RATE_SELECT, originalValue);
   
    

    
    // 5. Resumen de resultados
    SYS_CONSOLE_PRINT("\r\n=== Diagnostic Summary ===\r\n");
    SYS_CONSOLE_PRINT("Register reads successful: %d/8\r\n", successCount);
    SYS_CONSOLE_PRINT("Write/Read cycles successful: %d/%d\r\n", writeReadSuccess, testCount);
    
    // Diagnóstico
    if (successCount == 0 && writeReadSuccess == 0)
    {
        SYS_CONSOLE_PRINT("DIAGNOSIS: No communication - Check I2C connections and address\r\n");
    }
    else if (successCount > 0 && writeReadSuccess == 0)
    {
        SYS_CONSOLE_PRINT("DIAGNOSIS: Read works, Write fails - Check write enable/protection\r\n");
    }
    else if (writeReadSuccess > 0 && writeReadSuccess < testCount)
    {
        SYS_CONSOLE_PRINT("DIAGNOSIS: Partial communication - Possible timing or signal integrity issues\r\n");
    }
    else if (writeReadSuccess == testCount)
    {
        SYS_CONSOLE_PRINT("DIAGNOSIS: Communication is working correctly! �??\r\n");
    }
    
    SYS_CONSOLE_PRINT("=== Diagnostic Test Complete ===\r\n\r\n");
}
// Function to test codec functionality
void CODEC_TestFunction(void)
{
    CodecGain_t codecGain;
    CodecResult_t result;
    uint8_t regValue;
    
    SYS_CONSOLE_PRINT("\r\n=== CODEC Test Started ===\r\n");
    
    // Test the delay system
    SYS_CONSOLE_PRINT("Testing delay system (2 second delay)...\r\n");
    SYSTICK_DelayMs(2000);
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
    SYSTICK_TimerStart();
    PWR_LDOON_Set();
    PWR_BON_Set();
    PWR_HPWR_Set();
    PWR_SUSP_Clear();
    PWR_STBY_Clear();
    COD_RESET_Set();
    // Enable ADC1
    ADC1_Enable();

    // Register callback function for CH0 period interrupt
    TC0_TimerCallbackRegister(TC0_CH0_TimerInterruptHandler, (uintptr_t)NULL);

    // Start the timer channel 0
    TC0_TimerStart();
    int try = 0;
    while ( true )
    {
        // Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks();
        // Run codec test once after system initialization
        if (!codecTestDone && try>10)
        {
            // Wait for system to stabilize (2 seconds)
            I2C_ScanDevices();  // Scan I2C bus first
            CODEC_DiagnosticTest();
            codecTestDone = true;
            
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
        //SYSTICK_DelayMs(1000);
        try++;
    }

    // Execution should not come here during normal operation
    return EXIT_FAILURE;
}

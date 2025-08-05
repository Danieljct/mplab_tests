/******************************************************************************
* Title                 :   Codec configuration 
* Filename              :   codec.c
* Author                :   Daniel
* Origin Date           :   28/07/2025
* Version               :   1.0.0
* Target                :   SAMD51J20A
* Notes                 :   Adapted for MPLAB Harmony I2C Driver
*
******************************************************************************/

/******************************************************************************
**      HEADER FILES
******************************************************************************/
#include "codec.h"
#include "definitions.h"
#include "config/default/configuration.h"
#include <stdio.h>

/******************************************************************************
**      MODULE PREPROCESSOR CONSTANTS
******************************************************************************/
#define CODEC_I2C_TIMEOUT_MS    1000
#define I2C_TRANSFER_DELAY_MS   10

// I2C Driver Index (if not defined in configuration)
#ifndef DRV_I2C_INDEX_0
#define DRV_I2C_INDEX_0         0
#endif

/******************************************************************************
**      MODULE MACROS
******************************************************************************/

/******************************************************************************
*       MODULE DATATYPES
******************************************************************************/

/******************************************************************************
**      MODULE VARIABLE DEFINITIONS
******************************************************************************/
static DRV_HANDLE codecI2CHandle = DRV_HANDLE_INVALID;
static bool isCodecInitialized = false;

/******************************************************************************
**      PRIVATE FUNCTION DECLARATIONS (PROTOTYPES)
******************************************************************************/

static bool CODEC_waitForTransferComplete(void);

/******************************************************************************
**      FUNCTION DEFINITIONS
******************************************************************************/

/**
 * @brief Initialize the codec driver
 */
CodecResult_t CODEC_init(CodecGain_t gain)
{
    DRV_I2C_TRANSFER_SETUP transferSetup;
    
    // Open I2C driver
    codecI2CHandle = DRV_I2C_Open(DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE);
    
    if (codecI2CHandle == DRV_HANDLE_INVALID)
    {
        return CODEC_ERROR_I2C_INIT;
    }
    
    // Configure I2C transfer settings for 100kHz
    transferSetup.clockSpeed = 100000;
    if (!DRV_I2C_TransferSetup(codecI2CHandle, &transferSetup))
    {
        DRV_I2C_Close(codecI2CHandle);
        return CODEC_ERROR_I2C_INIT;
    }
    
    // Hardware reset
    if (CODEC_resetHw() != CODEC_SUCCESS)
    {
        DRV_I2C_Close(codecI2CHandle);
        return CODEC_ERROR_I2C_INIT;
    }
    
    // Software reset
    if (CODEC_writeRegister(CODEC_REG_SOFT_RESET, 0x01) != CODEC_SUCCESS)
    {
        DRV_I2C_Close(codecI2CHandle);
        return CODEC_ERROR_I2C_INIT;
    }
    
    // Wait for reset to complete
    SYSTICK_DelayMs(100);
    
    // Configure PLL if needed
   if (CODEC_configPll(gain) != CODEC_SUCCESS)
    {
        DRV_I2C_Close(codecI2CHandle);
        return CODEC_ERROR_I2C_INIT;
    }
    
    // Set initial gain
    if (CODEC_setGain(gain) != CODEC_SUCCESS)
    {
        DRV_I2C_Close(codecI2CHandle);
        return CODEC_ERROR_I2C_INIT;
    }
   
    isCodecInitialized = true;
    return CODEC_SUCCESS;
}

/**
 * @brief Write a value to a codec register
 */
CodecResult_t CODEC_writeRegister(uint8_t reg_add, uint8_t reg_val)
{
    uint8_t writeBuffer[2];
    DRV_I2C_TRANSFER_HANDLE transferHandle;
    
    if (!isCodecInitialized && codecI2CHandle == DRV_HANDLE_INVALID)
    {
        return CODEC_ERROR_I2C_INIT;
    }
    
    writeBuffer[0] = reg_add;
    writeBuffer[1] = reg_val;
    
    // Start write transfer
    DRV_I2C_WriteTransfer(codecI2CHandle, CODEC_ADDRESS, writeBuffer, 2);
    
    if (transferHandle == DRV_I2C_TRANSFER_HANDLE_INVALID)
    {
        return CODEC_ERROR_I2C_WRITE;
    }
    
    // Wait for transfer to complete
    if (!CODEC_waitForTransferComplete())
    {
        return CODEC_ERROR_TIMEOUT;
    }
    
    SYSTICK_DelayMs(I2C_TRANSFER_DELAY_MS);
    return CODEC_SUCCESS;
}

/**
 * @brief Read a value from a codec register
 */
CodecResult_t CODEC_readRegister(uint8_t reg_add, uint8_t* reg_val)
{
    DRV_I2C_TRANSFER_HANDLE transferHandle;
    
    if (!isCodecInitialized && codecI2CHandle == DRV_HANDLE_INVALID)
    {
        return CODEC_ERROR_I2C_INIT;
    }
    
    if (reg_val == NULL)
    {
        return CODEC_ERROR_I2C_READ;
    }
    
    // Write register address then read value
    DRV_I2C_WriteReadTransfer(codecI2CHandle, CODEC_ADDRESS, 
                                 &reg_add, 1, reg_val, 1);
    
    if (transferHandle == DRV_I2C_TRANSFER_HANDLE_INVALID)
    {
        return CODEC_ERROR_I2C_READ;
    }
    
    // Wait for transfer to complete
    if (!CODEC_waitForTransferComplete())
    {
        return CODEC_ERROR_TIMEOUT;
    }
    
    SYSTICK_DelayMs(I2C_TRANSFER_DELAY_MS);
    return CODEC_SUCCESS;
}

/**
 * @brief Set codec gain values
 */
CodecResult_t CODEC_setGain(CodecGain_t gain)
{
    CodecResult_t result;
    
    // Set left ADC PGA gain
    result = CODEC_writeRegister(CODEC_REG_LEFT_ADC_PGA_GAIN_CTRL, gain.leftGain);
    if (result != CODEC_SUCCESS)
        return result;
    
    // Set right ADC PGA gain
    result = CODEC_writeRegister(CODEC_REG_RIGHT_ADC_PGA_GAIN_CTRL, gain.rightGain);
    if (result != CODEC_SUCCESS)
        return result;
    
    // Set microphone bias control
    result = CODEC_writeRegister(CODEC_REG_MICBIAS_CTRL, gain.micGain);
    if (result != CODEC_SUCCESS)
        return result;
    
    return CODEC_SUCCESS;
}

/**
 * @brief Configure codec PLL
 */
CodecResult_t CODEC_configPll(CodecGain_t gain)
{
    CodecResult_t result;
    
    // Configure sample rate (example: 48kHz)
    result = CODEC_writeRegister(CODEC_REG_SAMPLE_RATE_SELECT, 0x00);
    if (result != CODEC_SUCCESS)
        return result;
    
    // Configure data path setup
    result = CODEC_writeRegister(CODEC_REG_DATA_PATH_SETUP, 0x0A);
    if (result != CODEC_SUCCESS)
        return result;
    
    // Configure serial data interface
    result = CODEC_writeRegister(CODEC_REG_SERIAL_DATA_INTERFACE_CTRL_A, 0x00);
    if (result != CODEC_SUCCESS)
        return result;
    
    result = CODEC_writeRegister(CODEC_REG_SERIAL_DATA_INTERFACE_CTRL_B, 0x00);
    if (result != CODEC_SUCCESS)
        return result;
    
    // Configure digital filter
    result = CODEC_writeRegister(CODEC_REG_DIGITAL_FILTER_CTRL, 0x00);
    if (result != CODEC_SUCCESS)
        return result;
    
    return CODEC_SUCCESS;
}

/**
 * @brief Print all codec register values for debugging
 */
CodecResult_t CODEC_printAllRegisters(void)
{
    uint8_t regValue;
    CodecResult_t result;
    
    // Array of important registers to read
    uint8_t registers[] = {
        CODEC_REG_PAGE_SELECT,
        CODEC_REG_SOFT_RESET,
        CODEC_REG_SAMPLE_RATE_SELECT,
        CODEC_REG_PLL_A,
        CODEC_REG_PLL_B,
        CODEC_REG_PLL_C,
        CODEC_REG_PLL_D,
        CODEC_REG_DATA_PATH_SETUP,
        CODEC_REG_SERIAL_DATA_INTERFACE_CTRL_A,
        CODEC_REG_SERIAL_DATA_INTERFACE_CTRL_B,
        CODEC_REG_SERIAL_DATA_INTERFACE_CTRL_C,
        CODEC_REG_OVERFLOW_FLAG,
        CODEC_REG_DIGITAL_FILTER_CTRL,
        CODEC_REG_LEFT_ADC_PGA_GAIN_CTRL,
        CODEC_REG_RIGHT_ADC_PGA_GAIN_CTRL,
        CODEC_REG_MIC2LR_LEFT_ADC_CTRL,
        CODEC_REG_MIC2LR_RIGHT_ADC_CTRL,
        CODEC_REG_MIC1LP_LINE1LP_LEFT_ADC_CTRL,
        CODEC_REG_LEFT_CHANNEL_ANALOG_INPUT_CM_CONNECTION,
        CODEC_REG_MIC1RP_LINE1RP_LEFT_ADC_CTRL,
        CODEC_REG_MIC1RP_LINE1RP_RIGHT_ADC_CTRL,
        CODEC_REG_RIGHT_CHANNEL_ANALOG_INPUT_CM_CONNECTION,
        CODEC_REG_MIC1LP_LINE1LP_RIGHT_ADC_CTRL,
        CODEC_REG_MICBIAS_CTRL
    };
    
    SYS_CONSOLE_PRINT("\r\n=== CODEC Register Dump ===\r\n");
    SYS_CONSOLE_PRINT("Address | Value | Description\r\n");
    SYS_CONSOLE_PRINT("--------|-------|------------\r\n");
    
    for (int i = 0; i < sizeof(registers); i++)
    {
        result = CODEC_readRegister(registers[i], &regValue);
        if (result == CODEC_SUCCESS)
        {
            SYS_CONSOLE_PRINT("  0x%02X  | 0x%02X  | ", registers[i], regValue);
            
            // Add descriptive names for important registers
            switch (registers[i])
            {
                case CODEC_REG_PAGE_SELECT:
                    SYS_CONSOLE_PRINT("Page Select\r\n");
                    break;
                case CODEC_REG_SOFT_RESET:
                    SYS_CONSOLE_PRINT("Soft Reset\r\n");
                    break;
                case CODEC_REG_SAMPLE_RATE_SELECT:
                    SYS_CONSOLE_PRINT("Sample Rate Select\r\n");
                    break;
                case CODEC_REG_LEFT_ADC_PGA_GAIN_CTRL:
                    SYS_CONSOLE_PRINT("Left ADC PGA Gain\r\n");
                    break;
                case CODEC_REG_RIGHT_ADC_PGA_GAIN_CTRL:
                    SYS_CONSOLE_PRINT("Right ADC PGA Gain\r\n");
                    break;
                case CODEC_REG_MICBIAS_CTRL:
                    SYS_CONSOLE_PRINT("Microphone Bias Control\r\n");
                    break;
                default:
                    SYS_CONSOLE_PRINT("Register %d\r\n", registers[i]);
                    break;
            }
        }
        else
        {
            SYS_CONSOLE_PRINT("  0x%02X  | ERROR | Failed to read register\r\n", registers[i]);
        }
    }
    
    SYS_CONSOLE_PRINT("=== End Register Dump ===\r\n\r\n");
    return CODEC_SUCCESS;
}

/******************************************************************************
**      PRIVATE FUNCTION DEFINITIONS
******************************************************************************/

/**
 * @brief Hardware reset of the codec (placeholder)
 */
CodecResult_t CODEC_resetHw(void)
{
    // Configure COD_RESET pin as output
    COD_RESET_OutputEnable();
    
    // Pull reset pin low for 10ms to perform hardware reset
    COD_RESET_Clear();
    SYSTICK_DelayMs(10);
    
    // Release reset pin (set high) to allow normal operation
    COD_RESET_Set();
    SYSTICK_DelayMs(5);  // Additional delay for codec to stabilize
    
    return CODEC_SUCCESS;
}

/**
 * @brief Wait for I2C transfer to complete
 */
static bool CODEC_waitForTransferComplete(void)
{
    uint32_t timeout = CODEC_I2C_TIMEOUT_MS;
    
    // This is a simplified timeout mechanism
    // In a real implementation, you would check the transfer status
    while (timeout > 0)
    {
        SYSTICK_DelayMs(1);
        timeout--;
        
        // For this example, we'll assume the transfer completes quickly
        if (timeout < (CODEC_I2C_TIMEOUT_MS - 10))
        {
            return true;
        }
    }
    
    return false;
}

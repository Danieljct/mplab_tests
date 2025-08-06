/******************************************************************************
* Title                 :   Codec configuration
* Filename              :   codec.h
* Author                :   Daniel
* Origin Date           :   28/07/2025
* Version               :   1.0.0
* Target                :   SAMD51J20A
* Notes                 :   Adapted for MPLAB Harmony I2C Driver
*
******************************************************************************/

#ifndef CODEC_H_
#define CODEC_H_

/******************************************************************************
**      INCLUDES
******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "definitions.h"

/******************************************************************************
**      PREPROCESSOR CONSTANTS
******************************************************************************/
#ifndef CODEC_DRIVER_CFG
#define CODEC_DRIVER_CFG

#define USE_CODEC_PLL 0

#endif

/******************************************************************************
**      CONFIGURATION CONSTANTS
******************************************************************************/

/******************************************************************************
**      MACRO DEFINITIONS
******************************************************************************/
#define CODEC_ADDRESS 0x18

/* REGISTER MAP */
#define CODEC_REG_PAGE_SELECT									0
#define CODEC_REG_SOFT_RESET									1
#define CODEC_REG_SAMPLE_RATE_SELECT							2
#define CODEC_REG_PLL_A											3
#define CODEC_REG_PLL_B											4
#define CODEC_REG_PLL_C											5
#define CODEC_REG_PLL_D											6
#define CODEC_REG_DATA_PATH_SETUP								7
#define CODEC_REG_SERIAL_DATA_INTERFACE_CTRL_A					8
#define CODEC_REG_SERIAL_DATA_INTERFACE_CTRL_B					9
#define CODEC_REG_SERIAL_DATA_INTERFACE_CTRL_C					10
#define CODEC_REG_OVERFLOW_FLAG									11
#define CODEC_REG_DIGITAL_FILTER_CTRL							12
#define CODEC_REG_LEFT_ADC_PGA_GAIN_CTRL						15
#define CODEC_REG_RIGHT_ADC_PGA_GAIN_CTRL						16
#define CODEC_REG_MIC2LR_LEFT_ADC_CTRL							17
#define CODEC_REG_MIC2LR_RIGHT_ADC_CTRL							18
#define CODEC_REG_MIC1LP_LINE1LP_LEFT_ADC_CTRL					19
#define CODEC_REG_LEFT_CHANNEL_ANALOG_INPUT_CM_CONNECTION 		20
#define CODEC_REG_MIC1RP_LINE1RP_LEFT_ADC_CTRL					21
#define CODEC_REG_MIC1RP_LINE1RP_RIGHT_ADC_CTRL					22
#define CODEC_REG_RIGHT_CHANNEL_ANALOG_INPUT_CM_CONNECTION 		23
#define CODEC_REG_MIC1LP_LINE1LP_RIGHT_ADC_CTRL					24
#define CODEC_REG_MICBIAS_CTRL									25
#define CODEC_REG_CLOCK											101
#define CODEC_REG_CLOCK_GEN_CTRL								102

/*****************************************************************************
**      PUBLIC DATATYPES														
*****************************************************************************/
typedef struct CodecGain
{
    uint8_t leftGain;
    uint8_t rightGain;
    uint8_t micGain;
} CodecGain_t;

typedef enum
{
    CODEC_SUCCESS = 0,
    CODEC_ERROR_I2C_INIT,
    CODEC_ERROR_I2C_WRITE,
    CODEC_ERROR_I2C_READ,
    CODEC_ERROR_TIMEOUT
} CodecResult_t;

/*****************************************************************************
**      PUBLIC (GLOBAL) VARIABLES DECLARATIONS
*****************************************************************************/

/******************************************************************************
**      PUBLIC FUNCTION PROTOTYPES
******************************************************************************/

/**
 * @brief Initialize the codec driver
 * @param gain Initial gain configuration
 * @return CODEC_SUCCESS if successful, error code otherwise
 */
CodecResult_t CODEC_init(CodecGain_t gain);

/**
 * @brief Set codec gain values
 * @param gain Gain configuration structure
 * @return CODEC_SUCCESS if successful, error code otherwise
 */
CodecResult_t CODEC_setGain(CodecGain_t gain);

/**
 * @brief Write a value to a codec register
 * @param reg_add Register address
 * @param reg_val Value to write
 * @return CODEC_SUCCESS if successful, error code otherwise
 */
CodecResult_t CODEC_writeRegister(uint8_t reg_add, uint8_t reg_val);

/**
 * @brief Read a value from a codec register
 * @param reg_add Register address
 * @param reg_val Pointer to store the read value
 * @return CODEC_SUCCESS if successful, error code otherwise
 */
CodecResult_t CODEC_readRegister(uint8_t reg_add, uint8_t* reg_val);

/**
 * @brief Configure codec PLL
 * @param gain Gain configuration
 * @return CODEC_SUCCESS if successful, error code otherwise
 */
CodecResult_t CODEC_configPll(CodecGain_t gain);

/**
 * @brief Print all codec register values for debugging
 * @return CODEC_SUCCESS if successful, error code otherwise
 */
CodecResult_t CODEC_printAllRegisters(void);

/**
 * @brief Initialize global timer for delay functionality (optional - called automatically)
 * @note This function initializes the global timer delay system
 * @return CODEC_SUCCESS if successful, error code otherwise
 */
CodecResult_t CODEC_initTimer(void);

/**
 * @brief Perform a hardware reset of the codec
 * @return CODEC_SUCCESS if successful, error code otherwise
 */
CodecResult_t CODEC_resetHw(void);

#ifdef __cplusplus
}
#endif

#endif // CODEC_H_

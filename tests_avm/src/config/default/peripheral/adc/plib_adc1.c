/*******************************************************************************
  Analog-to-Digital Converter(ADC1) PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_adc1.c

  Summary
    ADC1 PLIB Implementation File.

  Description
    This file defines the interface to the ADC peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

  Remarks:
    None.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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
/* This section lists the other files that are included in this file.
*/

#include "plib_adc1.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

#define ADC1_BIASCOMP_POS     (16U)
#define ADC1_BIASCOMP_Msk     (0x7UL << ADC1_BIASCOMP_POS)

#define ADC1_BIASREFBUF_POS   (19U)
#define ADC1_BIASREFBUF_Msk   (0x7UL << ADC1_BIASREFBUF_POS)

#define ADC1_BIASR2R_POS      (22U)
#define ADC1_BIASR2R_Msk      (0x7UL << ADC1_BIASR2R_POS)

// *****************************************************************************
// *****************************************************************************
// Section: ADC1 Implementation
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Initialize ADC module */
void ADC1_Initialize( void )
{
    /* Reset ADC */
    ADC1_REGS->ADC_CTRLA = ADC_CTRLA_SWRST_Msk;

    while((ADC1_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_SWRST_Msk) == ADC_SYNCBUSY_SWRST_Msk)
    {
        /* Wait for Synchronization */
    }

    /* Writing calibration values in BIASREFBUF, BIASCOMP and BIASR2R */
    ADC1_REGS->ADC_CALIB =(uint16_t)((ADC_CALIB_BIASCOMP((((*(uint32_t*)SW0_ADDR) & ADC1_BIASCOMP_Msk) >> ADC1_BIASCOMP_POS))) \
            | ADC_CALIB_BIASR2R((((*(uint32_t*)SW0_ADDR) & ADC1_BIASR2R_Msk) >> ADC1_BIASR2R_POS))
            | ADC_CALIB_BIASREFBUF(((*(uint32_t*)SW0_ADDR) & ADC1_BIASREFBUF_Msk)>> ADC1_BIASREFBUF_POS ));

    /* prescaler */
    ADC1_REGS->ADC_CTRLA = ADC_CTRLA_PRESCALER_DIV256;

    /* Sampling length */
    ADC1_REGS->ADC_SAMPCTRL = ADC_SAMPCTRL_SAMPLEN(0U) | ADC_SAMPCTRL_OFFCOMP_Msk;

    /* reference */
    ADC1_REGS->ADC_REFCTRL = ADC_REFCTRL_REFSEL_INTVCC1 | ADC_REFCTRL_REFCOMP_Msk;


    /* positive and negative input pins */
    ADC1_REGS->ADC_INPUTCTRL = (uint16_t) ADC_POSINPUT_AIN9 | (uint16_t) ADC_NEGINPUT_GND ;

    /* Resolution & Operation Mode */
    ADC1_REGS->ADC_CTRLB = ADC_CTRLB_RESSEL_12BIT | ADC_CTRLB_WINMODE(0U) | ADC_CTRLB_FREERUN_Msk;


    /* Clear all interrupt flags */
    ADC1_REGS->ADC_INTFLAG = ADC_INTFLAG_Msk;

    while(ADC1_REGS->ADC_SYNCBUSY != 0U)
    {
        /* Wait for Synchronization */
    }
}

/* Enable ADC module */
void ADC1_Enable( void )
{
    ADC1_REGS->ADC_CTRLA |= ADC_CTRLA_ENABLE_Msk;
    while((ADC1_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_ENABLE_Msk) == ADC_SYNCBUSY_ENABLE_Msk)
    {
        /* Wait for Synchronization */
    }
}

/* Disable ADC module */
void ADC1_Disable( void )
{
    ADC1_REGS->ADC_CTRLA &=(uint16_t) ~ADC_CTRLA_ENABLE_Msk;
    while((ADC1_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_ENABLE_Msk) == ADC_SYNCBUSY_ENABLE_Msk)
    {
        /* Wait for Synchronization */
    }
}

/* Configure channel input */
void ADC1_ChannelSelect( ADC_POSINPUT positiveInput, ADC_NEGINPUT negativeInput )
{
    /* Configure positive and negative input pins */
    uint16_t channel;
    channel = ADC1_REGS->ADC_INPUTCTRL;
    channel &= (uint16_t)~(ADC_INPUTCTRL_MUXPOS_Msk | ADC_INPUTCTRL_MUXNEG_Msk);
    channel |= (uint16_t) positiveInput | (uint16_t) negativeInput;
    ADC1_REGS->ADC_INPUTCTRL = channel;

    while((ADC1_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_INPUTCTRL_Msk) == ADC_SYNCBUSY_INPUTCTRL_Msk)
    {
        /* Wait for Synchronization */
    }
}

/* Start the ADC conversion by SW */
void ADC1_ConversionStart( void )
{
    /* Start conversion */
    ADC1_REGS->ADC_SWTRIG |= ADC_SWTRIG_START_Msk;

    while((ADC1_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_SWTRIG_Msk) == ADC_SYNCBUSY_SWTRIG_Msk)
    {
        /* Wait for Synchronization */
    }
}

/* Configure window comparison threshold values */
void ADC1_ComparisonWindowSet(uint16_t low_threshold, uint16_t high_threshold)
{
    ADC1_REGS->ADC_WINLT = low_threshold;
    ADC1_REGS->ADC_WINUT = high_threshold;
    while((ADC1_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_WINLT_Msk) == ADC_SYNCBUSY_WINLT_Msk)
    {
        /* Wait for Synchronization */
    }
    while((ADC1_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_WINUT_Msk) == ADC_SYNCBUSY_WINUT_Msk)
    {
        /* Wait for Synchronization */
    }
}

void ADC1_WindowModeSet(ADC_WINMODE mode)
{
    ADC1_REGS->ADC_CTRLB &= (uint16_t)~ADC_CTRLB_WINMODE_Msk;
    ADC1_REGS->ADC_CTRLB |= (uint16_t)mode << ADC_CTRLB_WINMODE_Pos;
    while((ADC1_REGS->ADC_SYNCBUSY & ADC_SYNCBUSY_CTRLB_Msk) == ADC_SYNCBUSY_CTRLB_Msk)
    {
        /* Wait for Synchronization */
    }
}

/* Read the conversion result */
uint16_t ADC1_ConversionResultGet( void )
{
    return (uint16_t)ADC1_REGS->ADC_RESULT;
}

/* Read the last conversion result */
uint16_t ADC1_LastConversionResultGet( void )
{
    return (uint16_t)ADC1_REGS->ADC_RESS;
}

void ADC1_InterruptsClear(ADC_STATUS interruptMask)
{
    ADC1_REGS->ADC_INTFLAG = interruptMask;
}

void ADC1_InterruptsEnable(ADC_STATUS interruptMask)
{
    ADC1_REGS->ADC_INTENSET = interruptMask;
}

void ADC1_InterruptsDisable(ADC_STATUS interruptMask)
{
    ADC1_REGS->ADC_INTENCLR = interruptMask;
}

/* Check whether result is ready */
bool ADC1_ConversionStatusGet( void )
{
    bool status;
    status =  (((ADC1_REGS->ADC_INTFLAG & ADC_INTFLAG_RESRDY_Msk) >> ADC_INTFLAG_RESRDY_Pos) != 0U);
    if (status == true)
    {
        /* Clear interrupt flag */
        ADC1_REGS->ADC_INTFLAG = ADC_INTFLAG_RESRDY_Msk;
    }
    return status;
}


/*******************************************************************************
  SERIAL COMMUNICATION SERIAL PERIPHERAL INTERFACE(SERCOM2_SPI) PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_sercom2_spi_slave.c

  Summary
    SERCOM2_SPI PLIB Slave Implementation File.

  Description
    This file defines the interface to the SERCOM SPI Slave peripheral library.
    This library provides access to and control of the associated
    peripheral instance.

  Remarks:
    None.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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


#include "interrupts.h"
#include "plib_sercom2_spi_slave.h"
#include <string.h>

// *****************************************************************************
// *****************************************************************************
// Section: MACROS Definitions
// *****************************************************************************
// *****************************************************************************
#define SERCOM2_SPI_READ_BUFFER_SIZE            256U
#define SERCOM2_SPI_WRITE_BUFFER_SIZE           256U

static volatile uint8_t SERCOM2_SPI_ReadBuffer[SERCOM2_SPI_READ_BUFFER_SIZE];
static volatile uint8_t SERCOM2_SPI_WriteBuffer[SERCOM2_SPI_WRITE_BUFFER_SIZE];


/* Global object to save SPI Exchange related data  */
static volatile SPI_SLAVE_OBJECT sercom2SPISObj;

// *****************************************************************************
// *****************************************************************************
// Section: SERCOM2_SPI Slave Implementation
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

static void mem_copy(volatile void* pDst, volatile void* pSrc, uint32_t nBytes)
{
    volatile uint8_t* pSource = (volatile uint8_t*)pSrc;
    volatile uint8_t* pDest = (volatile uint8_t*)pDst;

    for (uint32_t i = 0U; i < nBytes; i++)
    {
        pDest[i] = pSource[i];
    }
}

void SERCOM2_SPI_Initialize(void)
{
    /* CHSIZE - 8_BIT
     * PLOADEN - 1
     *  SSDE - 1 
     * RXEN - 1
     */
    SERCOM2_REGS->SPIS.SERCOM_CTRLB = SERCOM_SPIS_CTRLB_CHSIZE_8_BIT | SERCOM_SPIS_CTRLB_PLOADEN_Msk | SERCOM_SPIS_CTRLB_RXEN_Msk | SERCOM_SPIS_CTRLB_SSDE_Msk;

    /* Wait for synchronization */
    while(SERCOM2_REGS->SPIS.SERCOM_SYNCBUSY != 0U)
    {
        /* Do nothing */
    }

    /* Mode - SPI Slave
     * IBON - 1 (Set immediately upon buffer overflow)
     * DOPO - PAD0
     * DIPO - PAD0
     * CPHA - LEADING_EDGE
     * COPL - IDLE_LOW
     * DORD - MSB
     * ENABLE - 1
     */
    SERCOM2_REGS->SPIS.SERCOM_CTRLA = SERCOM_SPIS_CTRLA_MODE_SPI_SLAVE | SERCOM_SPIS_CTRLA_IBON_Msk | SERCOM_SPIS_CTRLA_DOPO_PAD0 | SERCOM_SPIS_CTRLA_DIPO_PAD0 | SERCOM_SPIS_CTRLA_CPOL_IDLE_LOW | SERCOM_SPIS_CTRLA_CPHA_LEADING_EDGE | SERCOM_SPIS_CTRLA_DORD_MSB | SERCOM_SPIS_CTRLA_ENABLE_Msk ;

    /* Wait for synchronization */
    while(SERCOM2_REGS->SPIS.SERCOM_SYNCBUSY != 0U)
    {
        /* Do nothing */
    }

    sercom2SPISObj.rdInIndex = 0U;
    sercom2SPISObj.wrOutIndex = 0U;
    sercom2SPISObj.nWrBytes = 0U;
    sercom2SPISObj.errorStatus = SPI_SLAVE_ERROR_NONE;
    sercom2SPISObj.callback = NULL ;
    sercom2SPISObj.transferIsBusy = false ;

    SERCOM2_REGS->SPIS.SERCOM_INTENSET = (uint8_t)(SERCOM_SPIS_INTENSET_SSL_Msk | SERCOM_SPIS_INTENCLR_RXC_Msk);

}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t SERCOM2_SPI_Read(void* pRdBuffer, size_t size)
{
    uint8_t intState = SERCOM2_REGS->SPIS.SERCOM_INTENSET;
    size_t rdSize = size;
    uint8_t* pDstBuffer = (uint8_t*)pRdBuffer;

    SERCOM2_REGS->SPIS.SERCOM_INTENCLR = intState;

    if (rdSize > sercom2SPISObj.rdInIndex)
    {
        rdSize = sercom2SPISObj.rdInIndex;
    }

    (void) mem_copy(pDstBuffer, SERCOM2_SPI_ReadBuffer, rdSize);

    SERCOM2_REGS->SPIS.SERCOM_INTENSET = intState;

    return rdSize;
}

/* For 9-bit mode, the "size" must be specified in terms of 16-bit words */
size_t SERCOM2_SPI_Write(void* pWrBuffer, size_t size )
{
    uint8_t intState = SERCOM2_REGS->SPIS.SERCOM_INTENSET;
    size_t wrSize = size;
    bool writeReady = false;
    uint32_t wrOutIndex = 0;
    uint8_t* pSrcBuffer = (uint8_t*)pWrBuffer;

    SERCOM2_REGS->SPIS.SERCOM_INTENCLR = intState;

    if (wrSize > SERCOM2_SPI_WRITE_BUFFER_SIZE)
    {
        wrSize = SERCOM2_SPI_WRITE_BUFFER_SIZE;
    }

   (void) mem_copy(SERCOM2_SPI_WriteBuffer, pSrcBuffer, wrSize);

    sercom2SPISObj.nWrBytes = wrSize;

    writeReady = (wrOutIndex < sercom2SPISObj.nWrBytes);
    writeReady = ((SERCOM2_REGS->SPIS.SERCOM_INTFLAG & SERCOM_SPIS_INTFLAG_DRE_Msk) == SERCOM_SPIS_INTFLAG_DRE_Msk) && writeReady;
    while (writeReady)
    {
        SERCOM2_REGS->SPIS.SERCOM_DATA = SERCOM2_SPI_WriteBuffer[wrOutIndex];
        wrOutIndex++;
        writeReady = (wrOutIndex < sercom2SPISObj.nWrBytes);
        writeReady = ((SERCOM2_REGS->SPIS.SERCOM_INTFLAG & SERCOM_SPIS_INTFLAG_DRE_Msk) == SERCOM_SPIS_INTFLAG_DRE_Msk) && writeReady;
    }

    sercom2SPISObj.wrOutIndex = wrOutIndex;

    /* Restore interrupt enable state and also enable DRE interrupt to start pre-loading of DATA register */
    SERCOM2_REGS->SPIS.SERCOM_INTENSET = (intState | (uint8_t)SERCOM_SPIS_INTENSET_DRE_Msk);

    return wrSize;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t SERCOM2_SPI_ReadCountGet(void)
{
    return sercom2SPISObj.rdInIndex;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t SERCOM2_SPI_ReadBufferSizeGet(void)
{
    return SERCOM2_SPI_READ_BUFFER_SIZE;
}

/* For 9-bit mode, the return value is in terms of 16-bit words */
size_t SERCOM2_SPI_WriteBufferSizeGet(void)
{
    return SERCOM2_SPI_WRITE_BUFFER_SIZE;
}

void SERCOM2_SPI_CallbackRegister(SERCOM_SPI_SLAVE_CALLBACK callBack, uintptr_t context )
{
    sercom2SPISObj.callback = callBack;

    sercom2SPISObj.context = context;
}

/* The status is returned busy during the period the chip-select remains asserted */
bool SERCOM2_SPI_IsBusy(void)
{
    return sercom2SPISObj.transferIsBusy;
}


SPI_SLAVE_ERROR SERCOM2_SPI_ErrorGet(void)
{
    SPI_SLAVE_ERROR errorStatus = sercom2SPISObj.errorStatus;

    sercom2SPISObj.errorStatus = SPI_SLAVE_ERROR_NONE;

    return errorStatus;
}

void __attribute__((used)) SERCOM2_SPI_InterruptHandler(void)
{
    uint8_t txRxData;
    uint8_t intFlag = SERCOM2_REGS->SPIS.SERCOM_INTFLAG;

    if((SERCOM2_REGS->SPIS.SERCOM_INTFLAG & SERCOM_SPIS_INTFLAG_SSL_Msk) == SERCOM_SPIS_INTFLAG_SSL_Msk)
    {
        /* Clear the SSL flag and enable TXC interrupt */
        SERCOM2_REGS->SPIS.SERCOM_INTFLAG = (uint8_t)SERCOM_SPIS_INTFLAG_SSL_Msk;
        SERCOM2_REGS->SPIS.SERCOM_INTENSET = (uint8_t)SERCOM_SPIS_INTENSET_TXC_Msk;
        sercom2SPISObj.rdInIndex = 0U;
        sercom2SPISObj.transferIsBusy = true;
    }

    if ((SERCOM2_REGS->SPIS.SERCOM_STATUS & SERCOM_SPIS_STATUS_BUFOVF_Msk) == SERCOM_SPIS_STATUS_BUFOVF_Msk)
    {
        /* Save the error to report it to application later, when the transfer is complete (TXC = 1) */
        sercom2SPISObj.errorStatus = SERCOM_SPIS_STATUS_BUFOVF_Msk;

        /* Clear the status register */
        SERCOM2_REGS->SPIS.SERCOM_STATUS = SERCOM_SPIS_STATUS_BUFOVF_Msk;

        /* Flush out the received data until RXC flag is set */
        while((SERCOM2_REGS->SPIS.SERCOM_INTFLAG & SERCOM_SPIS_INTFLAG_RXC_Msk) == SERCOM_SPIS_INTFLAG_RXC_Msk)
        {
            /* Reading DATA register will also clear the RXC flag */
            txRxData = (uint8_t)SERCOM2_REGS->SPIS.SERCOM_DATA;
        }

        /* Clear the Error Interrupt Flag */
        SERCOM2_REGS->SPIS.SERCOM_INTFLAG = (uint8_t)SERCOM_SPIS_INTFLAG_ERROR_Msk;
    }

    if((SERCOM2_REGS->SPIS.SERCOM_INTFLAG & SERCOM_SPIS_INTFLAG_RXC_Msk) == SERCOM_SPIS_INTFLAG_RXC_Msk)
    {
        /* Reading DATA register will also clear the RXC flag */
        txRxData = (uint8_t)SERCOM2_REGS->SPIS.SERCOM_DATA;

        if (sercom2SPISObj.rdInIndex < SERCOM2_SPI_READ_BUFFER_SIZE)
        {
            uint32_t rdInIndex = sercom2SPISObj.rdInIndex;

            SERCOM2_SPI_ReadBuffer[rdInIndex] = txRxData;
            sercom2SPISObj.rdInIndex++;
        }
    }

    if((SERCOM2_REGS->SPIS.SERCOM_INTFLAG & SERCOM_SPIS_INTFLAG_DRE_Msk) == SERCOM_SPIS_INTFLAG_DRE_Msk)
    {
        uint32_t wrOutIndex = sercom2SPISObj.wrOutIndex;

        if (wrOutIndex < sercom2SPISObj.nWrBytes)
        {
            txRxData = SERCOM2_SPI_WriteBuffer[wrOutIndex];
            wrOutIndex++;

            /* Before writing to DATA register (which clears TXC flag), check if TXC flag is set */
            if((SERCOM2_REGS->SPIS.SERCOM_INTFLAG & SERCOM_SPIS_INTFLAG_TXC_Msk) == SERCOM_SPIS_INTFLAG_TXC_Msk)
            {
                intFlag = (uint8_t)SERCOM_SPIS_INTFLAG_TXC_Msk;
            }
            SERCOM2_REGS->SPIS.SERCOM_DATA = (uint32_t)txRxData;
        }
        else
        {
            /* Disable DRE interrupt. The last byte sent by the master will be shifted out automatically */
            SERCOM2_REGS->SPIS.SERCOM_INTENCLR = (uint8_t)SERCOM_SPIS_INTENCLR_DRE_Msk;
        }

        sercom2SPISObj.wrOutIndex = wrOutIndex;
    }

    if((intFlag & SERCOM_SPIS_INTFLAG_TXC_Msk) == SERCOM_SPIS_INTFLAG_TXC_Msk)
    {
        /* Clear TXC flag */
        SERCOM2_REGS->SPIS.SERCOM_INTFLAG = (uint8_t)SERCOM_SPIS_INTFLAG_TXC_Msk;

        sercom2SPISObj.transferIsBusy = false;

        sercom2SPISObj.wrOutIndex = 0U;
        sercom2SPISObj.nWrBytes = 0U;

        if(sercom2SPISObj.callback != NULL)
        {
            uintptr_t context = sercom2SPISObj.context;

            sercom2SPISObj.callback(context);
        }
    }
}

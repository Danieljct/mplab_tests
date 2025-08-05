#include "AVM_ble_cmd.h"
#include "definitions.h"

#define REPLY       true
#define NO_REPLY    false
#define FIFO_MAX    5
#define BASEYEAR    2000
#define AVM_VERSION 1
#define AVM_NUMBER  42

uint8_t *ble_rx_buff;
uint8_t *ble_tx_buff;
uint8_t *ble_tx_size;
static int file_number = 0;
static int file_count = 10; // Simulado

static volatile functionPointerType ble_fifo[FIFO_MAX];
static volatile uint8_t ble_fifoTail;
static volatile uint8_t ble_fifoHead;
static volatile uint8_t ble_fifoLen;

// Funciones privadas (solo imprimen)
static void AVM_ble_powerOff(void);
static void AVM_ble_getStatus(void);
static void AVM_ble_recStop(void);
static void AVM_ble_setLeftGain(void);
static void AVM_ble_setRightGain(void);
static void AVM_ble_getLeftGain(void);
static void AVM_ble_getRightGain(void);
static void AVM_ble_usbMode(void);
static void AVM_ble_getBatState(void);
static void AVM_ble_setDate(void);
static void AVM_ble_setTime(void);
static void AVM_ble_setUserID(void);
static void AVM_ble_updateUserInfo(void);
static void AVM_ble_getNumFiles(void);
static void AVM_ble_setSampleRate(void);
static void AVM_ble_getSampleRate(void);

int AVM_ble_addToFifo(functionPointerType const cmd)
{
    if (ble_fifoLen < FIFO_MAX)
    {
        ble_fifo[ble_fifoTail] = cmd;
        ble_fifoTail = (ble_fifoTail + 1) % FIFO_MAX;
        ble_fifoLen++;
        return 0;
    }
    else
    {
        return 1;
    }
}

int AVM_ble_getFromFifo(functionPointerType *cmd_ptr)
{
    if (ble_fifoLen > 0)
    {
        *cmd_ptr = ble_fifo[ble_fifoHead];
        ble_fifoHead = (ble_fifoHead + 1) % FIFO_MAX;
        ble_fifoLen--;
        return 0;
    }
    else
    {
        return 1;
    }
}

int AVM_ble_fifoLen()
{
    return ble_fifoLen;
}

bool AVM_ble_cmdHandler(enum avm_ble_cmd cmd)
{
    bool ret_val = false;
    switch (cmd)
    {
        case (BLE_POWER_OFF):
        {
            AVM_ble_addToFifo(AVM_ble_powerOff);
            break;
        }
        case (BLE_GET_DEVICE_STATUS):
        {
            AVM_ble_getStatus();
            *ble_tx_size = 1;
            ret_val = true;
            break;
        }
        case (BLE_NUMBER_FILES):
        {
            AVM_ble_getNumFiles();
            file_number = 0;
            *ble_tx_size = 1;
            ret_val = true;
            break;
        }
        case (BLE_REC_STOP):
        {
            AVM_ble_addToFifo(AVM_ble_recStop);
            break;
        }
        case (BLE_SET_LEFT_GAIN):
        {
            AVM_ble_addToFifo(AVM_ble_setLeftGain);
            break;
        }
        case (BLE_SET_RIGHT_GAIN):
        {
            AVM_ble_addToFifo(AVM_ble_setRightGain);
            break;
        }
        case (BLE_GET_LEFT_GAIN):
        {
            AVM_ble_getLeftGain();
            *ble_tx_size = 1;
            ret_val = true;
            break;
        }
        case (BLE_GET_RIGHT_GAIN):
        {
            AVM_ble_getRightGain();
            *ble_tx_size = 1;
            ret_val = true;
            break;
        }
        case (BLE_USB_MODE):
        {
            AVM_ble_addToFifo(AVM_ble_usbMode);
            break;
        }
        case (BLE_BAT_STATE):
        {
            AVM_ble_getBatState();
            *ble_tx_size = 1;
            ret_val = true;
            break;
        }
        case (BLE_SET_DATE):
        {
            AVM_ble_addToFifo(AVM_ble_setDate);
            break;
        }
        case (BLE_SET_TIME):
        {
            AVM_ble_addToFifo(AVM_ble_setTime);
            break;
        }
        case (BLE_SET_UINFO_ID):
        {
            AVM_ble_addToFifo(AVM_ble_setUserID);
            break;
        }
        case (BLE_UPDATE_UINFO):
        {
            AVM_ble_addToFifo(AVM_ble_updateUserInfo);
            break;
        }
        case (BLE_SET_SAMPLE_RATE):
        {
            AVM_ble_addToFifo(AVM_ble_setSampleRate);
            break;
        }
        case (BLE_GET_SAMPLE_RATE):
        {
            AVM_ble_getSampleRate();
            *ble_tx_size = 1;
            ret_val = true;
            break;
        }
        case (BLE_GET_VERSION):
        {
            *ble_tx_size = 1;
            ret_val = true;
            *ble_tx_buff = AVM_VERSION;
            SYS_CONSOLE_PRINT("Action: Get Version (returning %d)\r\n", AVM_VERSION);
            break;
        }
        case (BLE_GET_NUMBER):
        {
            *ble_tx_size = 1;
            ret_val = true;
            *ble_tx_buff = AVM_NUMBER;
            SYS_CONSOLE_PRINT("Action: Get Number (returning %d)\r\n", AVM_NUMBER);
            break;
        }
        default:
        {
            SYS_CONSOLE_PRINT("Action: Unknown command %d\r\n", cmd);
            break;
        }
    }
    return ret_val;
}

int AVM_ble_cmdInit(uint8_t *rx_b, uint8_t *tx_b, uint8_t *tx_size)
{
    ble_rx_buff = rx_b;
    ble_tx_buff = tx_b;
    ble_tx_size = tx_size;
    
    ble_fifoHead = 0;
    ble_fifoTail = 0;
    ble_fifoLen = 0;

    SYS_CONSOLE_PRINT("AVM BLE Command system initialized\r\n");
    return 0;
}

// Implementaciones que solo imprimen
static void AVM_ble_powerOff()
{
    SYS_CONSOLE_PRINT("Action: Power Off - Setting device to OFF state\r\n");
}

static void AVM_ble_getStatus()
{
    *ble_tx_buff = 0x01; // Estado simulado
    SYS_CONSOLE_PRINT("Action: Get Device Status (returning 0x01)\r\n");
}

static void AVM_ble_recStop()
{
    SYS_CONSOLE_PRINT("Action: Record Stop - Toggling recording state\r\n");
}

static void AVM_ble_setLeftGain()
{
    uint8_t gain = ble_rx_buff[1];
    SYS_CONSOLE_PRINT("Action: Set Left Gain to %d (0x%02X)\r\n", gain, gain);
}

static void AVM_ble_setRightGain()
{
    uint8_t gain = ble_rx_buff[1];
    SYS_CONSOLE_PRINT("Action: Set Right Gain to %d (0x%02X)\r\n", gain, gain);
}

static void AVM_ble_getLeftGain()
{
    *ble_tx_buff = 0x50; // Gain simulado
    SYS_CONSOLE_PRINT("Action: Get Left Gain (returning 0x50)\r\n");
}

static void AVM_ble_getRightGain()
{
    *ble_tx_buff = 0x60; // Gain simulado
    SYS_CONSOLE_PRINT("Action: Get Right Gain (returning 0x60)\r\n");
}

static void AVM_ble_usbMode()
{
    SYS_CONSOLE_PRINT("Action: USB Mode - Toggling USB/Serial mode\r\n");
}

static void AVM_ble_getBatState()
{
    uint8_t bat = 85; // 85% simulado
    *ble_tx_buff = bat;
    SYS_CONSOLE_PRINT("Action: Get Battery State (returning %d%%)\r\n", bat);
}

static void AVM_ble_setDate()
{
    uint8_t day = ble_rx_buff[1];
    uint8_t month = ble_rx_buff[2];
    uint16_t year = ((uint16_t)ble_rx_buff[4] << 8) | ble_rx_buff[3];
    SYS_CONSOLE_PRINT("Action: Set Date to %02d/%02d/%04d\r\n", day, month, year);
}

static void AVM_ble_setTime()
{
    uint8_t hour = ble_rx_buff[1];
    uint8_t min = ble_rx_buff[2];
    uint8_t sec = ble_rx_buff[3];
    SYS_CONSOLE_PRINT("Action: Set Time to %02d:%02d:%02d\r\n", hour, min, sec);
}

static void AVM_ble_setUserID()
{
    SYS_CONSOLE_PRINT("Action: Set User ID to: %s\r\n", (char*)ble_rx_buff+1);
}

static void AVM_ble_updateUserInfo()
{
    SYS_CONSOLE_PRINT("Action: Update User Info - Saving user information\r\n");
}

static void AVM_ble_getNumFiles(void)
{
    *ble_tx_buff = file_count;
    SYS_CONSOLE_PRINT("Action: Get Number of Files (returning %d)\r\n", file_count);
}

static void AVM_ble_setSampleRate(void)
{
    int new_rate = ble_rx_buff[1];
    SYS_CONSOLE_PRINT("Action: Set Sample Rate to %d kHz\r\n", new_rate);
}

static void AVM_ble_getSampleRate(void)
{
    *ble_tx_buff = 10; // 10 kHz simulado
    SYS_CONSOLE_PRINT("Action: Get Sample Rate (returning 10 kHz)\r\n");
}

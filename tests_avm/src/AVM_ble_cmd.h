#ifndef AVM_BLE_CMD_H
#define AVM_BLE_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"
#include <stdint.h>
#include <stdbool.h>

typedef void(*functionPointerType)(void);

enum avm_ble_cmd
{
    BLE_POWER_OFF = 1,
    BLE_GET_DEVICE_STATUS,
    BLE_REC_STOP,
    BLE_SET_LEFT_GAIN,
    BLE_GET_LEFT_GAIN,      //5
    BLE_SET_RIGHT_GAIN,
    BLE_GET_RIGHT_GAIN,
    BLE_USB_MODE,
    BLE_BAT_STATE,
    BLE_SET_DATE,           //10
    BLE_SET_TIME,
    BLE_DEV_NAME,
    BLE_UPDATE_UINFO,
    BLE_SET_UINFO_ID,
    BLE_SET_UINFO_FNAME,    //15
    BLE_SET_UINFO_LNAME,
    BLE_NUMBER_FILES,
    BLE_READ_FILES,
    BLE_SET_SAMPLE_RATE,
    BLE_GET_SAMPLE_RATE,
    BLE_MUTE_MIC,
    BLE_MUTE_ACC,
    BLE_GET_VERSION,
    BLE_GET_NUMBER,
};

int AVM_ble_cmdInit(uint8_t *rx_b, uint8_t *tx_b, uint8_t *tx_size);
int AVM_ble_addToFifo(functionPointerType const cmd);
int AVM_ble_getFromFifo(functionPointerType *cmd_ptr);
int AVM_ble_fifoLen();

bool AVM_ble_cmdHandler(enum avm_ble_cmd cmd);

#ifdef __cplusplus
}
#endif

#endif

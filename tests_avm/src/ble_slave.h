#ifndef BLE_SLAVE_H
#define BLE_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"
#include <stdint.h>
#include <stdbool.h>

#define BLE_BUFFER_SIZE     32

int BLE_slave_init(void);
void BLE_serviceRequest(void);
void send_file_list_to_nrf(int file_number);

#ifdef __cplusplus
}
#endif

#endif

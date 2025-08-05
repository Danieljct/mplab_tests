#include "ble_slave.h"
#include "AVM_ble_cmd.h"
#include "definitions.h"

uint8_t rx_buff[BLE_BUFFER_SIZE], tx_buff[BLE_BUFFER_SIZE];
uint8_t tx_len;
static uint8_t Bcount;
static bool resp;

// Simulación de lista de archivos
#define MAX_FILES 10
#define MAX_FILENAME_LEN 32
static char file_list[MAX_FILES][MAX_FILENAME_LEN] = {
    "audio_001.wav",
    "audio_002.wav", 
    "audio_003.wav",
    "recording_001.wav",
    "recording_002.wav",
    "test_audio.wav",
    "voice_memo_01.wav",
    "voice_memo_02.wav",
    "sample_audio.wav",
    "last_recording.wav"
};

int BLE_slave_init(void)
{
    SYS_CONSOLE_PRINT("BLE Slave initializing...\r\n");
    
    // Inicializar buffers
    Bcount = 0;
    resp = false;
    tx_len = 0;
    
    // Inicializar sistema de comandos
    AVM_ble_cmdInit(rx_buff, tx_buff, &tx_len);
    
    SYS_CONSOLE_PRINT("BLE Slave initialized successfully\r\n");
    return 0;
}

void BLE_serviceRequest()
{
    functionPointerType cmd = NULL;
    while (AVM_ble_fifoLen())
    {
        AVM_ble_getFromFifo(&cmd);
        if (cmd != NULL)
        {
            cmd();
        }
    }
}

void send_file_list_to_nrf(int file_n)
{
    if (file_n < MAX_FILES)
    {
        SYS_CONSOLE_PRINT("Action: Send file list entry %d: %s\r\n", 
                         file_n, file_list[file_n]);
        
        // Simular envío por SPI
        size_t len = strlen(file_list[file_n]);
        if (len < BLE_BUFFER_SIZE)
        {
            strcpy((char*)tx_buff, file_list[file_n]);
            tx_len = len;
            
            // Aquí iría SERCOM2_SPI_Write(tx_buff, tx_len);
            SYS_CONSOLE_PRINT("  -> Would send %d bytes via SPI\r\n", tx_len);
        }
    }
    else
    {
        SYS_CONSOLE_PRINT("Action: Send file list - invalid index %d\r\n", file_n);
    }
}

// Función para procesar comandos recibidos por SPI
bool BLE_processCommand(uint8_t cmd, uint8_t* rx_data, size_t rx_len)
{
    SYS_CONSOLE_PRINT("\r\n=== Processing BLE Command ===\r\n");
    SYS_CONSOLE_PRINT("Command: %d, Data length: %d\r\n", cmd, rx_len);
    
    // Copiar datos recibidos al buffer
    if (rx_len > 0 && rx_len <= BLE_BUFFER_SIZE)
    {
        memcpy(rx_buff, rx_data, rx_len);
        Bcount = rx_len;
    }
    
    // Procesar comando
    resp = AVM_ble_cmdHandler((enum avm_ble_cmd)cmd);
    
    if (resp)
    {
        SYS_CONSOLE_PRINT("Command requires response: %d bytes\r\n", tx_len + 1);
        // Aquí se enviaría la respuesta por SPI
        // SERCOM2_SPI_Write(tx_buff, tx_len + 1);
        
        SYS_CONSOLE_PRINT("Response data: ");
        for (int i = 0; i < tx_len; i++)
        {
            SYS_CONSOLE_PRINT("0x%02X ", tx_buff[i]);
        }
        SYS_CONSOLE_PRINT("\r\n");
        
        resp = false;
    }
    else
    {
        SYS_CONSOLE_PRINT("Command requires no immediate response\r\n");
    }
    
    // Procesar comandos en FIFO
    BLE_serviceRequest();
    
    SYS_CONSOLE_PRINT("=== Command Processing Complete ===\r\n\r\n");
    
    return true;
}

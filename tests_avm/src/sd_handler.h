#ifndef SD_HANDLER_H
#define SD_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"
#include <stdint.h>
#include <stdbool.h>

// Tipos de resultado para operaciones SD
typedef enum
{
    SD_SUCCESS = 0,
    SD_ERROR_INIT,
    SD_ERROR_MOUNT,
    SD_ERROR_WRITE,
    SD_ERROR_READ,
    SD_ERROR_FILE_OPEN,
    SD_ERROR_NOT_READY
} SD_Result_t;

// Inicializar sistema de SD Card
SD_Result_t SD_Initialize(void);

// Escribir datos a un archivo en SD
SD_Result_t SD_WriteFile(const char* filename, const char* data, size_t dataSize);

// Leer archivo desde SD
SD_Result_t SD_ReadFile(const char* filename, char* buffer, size_t bufferSize, size_t* bytesRead);

// Verificar si SD está montada y lista
bool SD_IsReady(void);

// Obtener información de la SD
void SD_PrintInfo(void);

// Listar archivos en directorio raíz
void SD_ListFiles(void);

// Función de prueba - escribir archivo de test
SD_Result_t SD_WriteTestFile(void);

// Obtener información del drive actual
SD_Result_t SD_GetCurrentDriveInfo(char* driveBuffer, size_t bufferSize);

#ifdef __cplusplus
}
#endif

#endif

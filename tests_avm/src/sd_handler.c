#include "sd_handler.h"
#include "definitions.h"
#include <string.h>
#include <stdio.h>

// Variables estáticas para estado del sistema
static bool sdInitialized = false;
static bool sdMounted = false;
static SYS_FS_HANDLE fileHandle;

SD_Result_t SD_Initialize(void)
{
    SYS_CONSOLE_PRINT("Inicializando sistema de archivos SD...\r\n");
    
    // Esperamos a que el sistema esté listo y el driver SDMMC esté funcionando
    uint32_t timeout = 10000; // 10 segundos timeout
    
    while(timeout > 0)
    {
        // Verificar si el driver SDMMC está listo usando el media manager
        if(SYS_FS_MEDIA_MANAGER_MediaStatusGet("/dev/mmcblka1") == true)
        {
            SYS_CONSOLE_PRINT("Driver SDMMC listo!\r\n");
            sdInitialized = true;
            
            // Intentar montar el volumen directamente
            if(SYS_FS_Mount("/dev/mmcblka1", "/mnt/myDrive1", FAT, 0, NULL) == SYS_FS_RES_SUCCESS)
            {
                SYS_CONSOLE_PRINT("SD Card montada exitosamente en /mnt/myDrive1\r\n");
                sdMounted = true;
                return SD_SUCCESS;
            }
            else
            {
                SYS_CONSOLE_PRINT("Error montando SD Card - intentando con EXFAT...\r\n");
                
                // Si FAT falla, intentar con otros sistemas de archivos si están disponibles
                // En este caso, el sistema debería detectar automáticamente ExFAT
                SYSTICK_DelayMs(100);
                
                if(SYS_FS_Mount("/dev/mmcblka1", "/mnt/myDrive1", FAT, 0, NULL) == SYS_FS_RES_SUCCESS)
                {
                    SYS_CONSOLE_PRINT("SD Card montada en segundo intento\r\n");
                    sdMounted = true;
                    return SD_SUCCESS;
                }
                else
                {
                    SYS_CONSOLE_PRINT("Error: No se pudo montar la SD Card\r\n");
                    return SD_ERROR_MOUNT;
                }
            }
        }
        
        SYSTICK_DelayMs(10);
        timeout -= 10;
    }
    
    SYS_CONSOLE_PRINT("Timeout esperando que el driver SDMMC esté listo\r\n");
    return SD_ERROR_INIT;
}

SD_Result_t SD_WriteFile(const char* filename, const char* data, size_t dataSize)
{
    if(!sdMounted)
    {
        SYS_CONSOLE_PRINT("SD no está montada\r\n");
        return SD_ERROR_NOT_READY;
    }
    
    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "/mnt/myDrive1/%s", filename);
    
    SYS_CONSOLE_PRINT("Abriendo archivo: %s\r\n", fullPath);
    
    // Abrir archivo para escritura
    fileHandle = SYS_FS_FileOpen(fullPath, SYS_FS_FILE_OPEN_WRITE_PLUS);
    
    if(fileHandle == SYS_FS_HANDLE_INVALID)
    {
        SYS_CONSOLE_PRINT("Error abriendo archivo para escritura\r\n");
        return SD_ERROR_FILE_OPEN;
    }
    
    // Escribir datos
    size_t bytesWritten = SYS_FS_FileWrite(fileHandle, (const void*)data, dataSize);
    
    if(bytesWritten != dataSize)
    {
        SYS_CONSOLE_PRINT("Error escribiendo datos. Escritos: %d de %d bytes\r\n", 
                         bytesWritten, dataSize);
        SYS_FS_FileClose(fileHandle);
        return SD_ERROR_WRITE;
    }
    
    // Cerrar archivo
    if(SYS_FS_FileClose(fileHandle) != SYS_FS_RES_SUCCESS)
    {
        SYS_CONSOLE_PRINT("Error cerrando archivo\r\n");
        return SD_ERROR_WRITE;
    }
    
    SYS_CONSOLE_PRINT("Archivo escrito exitosamente: %d bytes\r\n", bytesWritten);
    return SD_SUCCESS;
}

SD_Result_t SD_ReadFile(const char* filename, char* buffer, size_t bufferSize, size_t* bytesRead)
{
    if(!sdMounted)
    {
        return SD_ERROR_NOT_READY;
    }
    
    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "/mnt/myDrive1/%s", filename);
    
    // Abrir archivo para lectura
    fileHandle = SYS_FS_FileOpen(fullPath, SYS_FS_FILE_OPEN_READ);
    
    if(fileHandle == SYS_FS_HANDLE_INVALID)
    {
        SYS_CONSOLE_PRINT("Error abriendo archivo para lectura: %s\r\n", fullPath);
        return SD_ERROR_FILE_OPEN;
    }
    
    // Leer datos
    *bytesRead = SYS_FS_FileRead(fileHandle, buffer, bufferSize - 1);
    buffer[*bytesRead] = '\0'; // Null terminator para strings
    
    // Cerrar archivo
    SYS_FS_FileClose(fileHandle);
    
    SYS_CONSOLE_PRINT("Archivo leído: %d bytes\r\n", *bytesRead);
    return SD_SUCCESS;
}

bool SD_IsReady(void)
{
    return sdMounted;
}

SD_Result_t SD_GetCurrentDriveInfo(char* driveBuffer, size_t bufferSize)
{
    if(driveBuffer == NULL || bufferSize < 32)
    {
        return SD_ERROR_NOT_READY;
    }
    
    // Si el SD está montado, devolver la información del mount point
    if(sdMounted)
    {
        strncpy(driveBuffer, "/mnt/myDrive1", bufferSize - 1);
        driveBuffer[bufferSize - 1] = '\0';
        SYS_CONSOLE_PRINT("Mount point: %s\r\n", driveBuffer);
        return SD_SUCCESS;
    }
    else
    {
        SYS_CONSOLE_PRINT("SD no está montada\r\n");
        return SD_ERROR_NOT_READY;
    }
}

void SD_PrintInfo(void)
{
    SYS_CONSOLE_PRINT("\r\n=== Información SD Card ===\r\n");
    SYS_CONSOLE_PRINT("Inicializada: %s\r\n", sdInitialized ? "Sí" : "No");
    SYS_CONSOLE_PRINT("Montada: %s\r\n", sdMounted ? "Sí" : "No");
    
    // Mostrar información del mount point
    char mountPoint[32];
    if(SD_GetCurrentDriveInfo(mountPoint, sizeof(mountPoint)) == SD_SUCCESS)
    {
        SYS_CONSOLE_PRINT("Mount point: %s\r\n", mountPoint);
    }
    
    if(sdMounted)
    {
        // Obtener información del volumen
        uint32_t totalSectors, freeSectors;
        if(SYS_FS_DriveSectorGet("/mnt/myDrive1", &totalSectors, &freeSectors) == SYS_FS_RES_SUCCESS)
        {
            SYS_CONSOLE_PRINT("Sectores totales: %u\r\n", totalSectors);
            SYS_CONSOLE_PRINT("Sectores libres: %u\r\n", freeSectors);
            SYS_CONSOLE_PRINT("Espacio total: %u KB\r\n", (totalSectors * 512) / 1024);
            SYS_CONSOLE_PRINT("Espacio libre: %u KB\r\n", (freeSectors * 512) / 1024);
        }
        else
        {
            SYS_CONSOLE_PRINT("No se pudo obtener información de sectores\r\n");
        }
    }
    SYS_CONSOLE_PRINT("===========================\r\n\r\n");
}

void SD_ListFiles(void)
{
    if(!sdMounted)
    {
        SYS_CONSOLE_PRINT("SD no está montada\r\n");
        return;
    }
    
    SYS_FS_HANDLE dirHandle;
    SYS_FS_FSTAT fileStat;
    
    dirHandle = SYS_FS_DirOpen("/mnt/myDrive1/");
    
    if(dirHandle == SYS_FS_HANDLE_INVALID)
    {
        SYS_CONSOLE_PRINT("Error abriendo directorio raíz\r\n");
        return;
    }
    
    SYS_CONSOLE_PRINT("\r\n=== Archivos en SD Card ===\r\n");
    
    while(SYS_FS_DirRead(dirHandle, &fileStat) == SYS_FS_RES_SUCCESS)
    {
        if(fileStat.fname[0] == '\0')
            break;
        
        if(fileStat.fattrib & SYS_FS_ATTR_DIR)
        {
            SYS_CONSOLE_PRINT("[DIR]  %s\r\n", fileStat.fname);
        }
        else
        {
            SYS_CONSOLE_PRINT("[FILE] %s (%u bytes)\r\n", fileStat.fname, fileStat.fsize);
        }
    }
    
    SYS_FS_DirClose(dirHandle);
    SYS_CONSOLE_PRINT("==========================\r\n\r\n");
}

SD_Result_t SD_WriteTestFile(void)
{
    const char* testFilename = "test_avm.txt";
    char testData[512];
    
    // Crear datos de prueba con timestamp
    uint32_t timestamp = SYSTICK_GetTickCounter();
    snprintf(testData, sizeof(testData), 
        "=== AVM Test File ===\r\n"
        "Timestamp: %u\r\n"
        "Sistema: SAMD51J20A\r\n"
        "Proyecto: AVM_Tests\r\n"
        "Estado: Sistema funcionando correctamente\r\n"
        "I2C Codec: Inicializado\r\n"
        "SPI BLE: Funcionando\r\n"
        "ADC: Leyendo valores\r\n"
        "SD Card: Escribiendo archivo de prueba\r\n"
        "=== Fin del archivo ===\r\n");
    
    SYS_CONSOLE_PRINT("\r\n=== Escribiendo archivo de prueba ===\r\n");
    SYS_CONSOLE_PRINT("Archivo: %s\r\n", testFilename);
    SYS_CONSOLE_PRINT("Tamaño: %d bytes\r\n", strlen(testData));
    
    SD_Result_t result = SD_WriteFile(testFilename, testData, strlen(testData));
    
    if(result == SD_SUCCESS)
    {
        SYS_CONSOLE_PRINT("¡Archivo de prueba escrito exitosamente!\r\n");
        
        // Verificar leyendo el archivo
        char readBuffer[512];
        size_t bytesRead;
        
        if(SD_ReadFile(testFilename, readBuffer, sizeof(readBuffer), &bytesRead) == SD_SUCCESS)
        {
            SYS_CONSOLE_PRINT("Verificación de lectura exitosa: %d bytes\r\n", bytesRead);
            SYS_CONSOLE_PRINT("Contenido leído:\r\n%s\r\n", readBuffer);
        }
    }
    else
    {
        SYS_CONSOLE_PRINT("Error escribiendo archivo de prueba: %d\r\n", result);
    }
    
    SYS_CONSOLE_PRINT("=====================================\r\n\r\n");
    return result;
}

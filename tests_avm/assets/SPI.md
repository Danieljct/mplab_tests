# Configuración de SPI (Modo Esclavo)

## 1. Introducción

El AVM utiliza **SPI en modo esclavo** para comunicarse con el NRF52830.  
Esta guía describe cómo configurar el SPI slave.

---

## 2. Agregar el SERCOMx

Para agregar el periférico SERCOMx, consulta la guía:  
[Configuración de I2C (SERCOM)](./I2C.md)

---

## 3. Configuración del SERCOMx como SPI Slave

En las opciones de configuración del SERCOMx, selecciona el modo **SPI Slave**:

<div align="center">
	<img src="images/SPI.png" alt="SPI driver" width="700"/>
</div>

> ⚙️ **Nota:** Las configuraciones específicas dependerán del periférico y la aplicación.

---

## 4. Configuración de Pines

Configura los pines correspondientes en el *Pin Configurator*:

<div align="center">
	<img src="images/pines_spi.png" alt="SPI driver" width="900"/>
</div>

> 🔔 **Asegúrate de que el periférico tenga un reloj (clk) asignado.**

---

## 5. Funciones Disponibles

Al generar el código, estarán disponibles funciones en:

```
src/config/default/peripheral/sercom/spi_slave/plib_sercom2_spi_slave.h
```

| Función                                 | Descripción                                 |
|------------------------------------------|---------------------------------------------|
| `SERCOM2_SPI_Initialize()`               | Inicializa el SPI slave                     |
| `SERCOM2_SPI_CallbackRegister(...)`      | Registra el callback de SPI                 |
| `SERCOM2_SPI_Read(...)`                  | Lee datos recibidos                         |
| `SERCOM2_SPI_ReadCountGet()`             | Obtiene la cantidad de bytes recibidos      |
| `SERCOM2_SPI_ErrorGet()`                 | Obtiene el estado de error                  |

---

## 6. Ejemplo de Uso en main.c

```c
// Callback para cuando se completa una transacción SPI
void SPI_SlaveCallback(uintptr_t context)
{
    // Verificar si hay errores primero
    SPI_SLAVE_ERROR error = SERCOM2_SPI_ErrorGet();
    if (error != SPI_SLAVE_ERROR_NONE)
    {
        return;
    }
    
    // Obtener cuántos bytes fueron recibidos
    lastBytesReceived = SERCOM2_SPI_ReadCountGet();
    
    if (lastBytesReceived > 0)
    {
        // Leer los datos recibidos
        size_t bytesRead = SERCOM2_SPI_Read(spiRxBuffer, sizeof(spiRxBuffer));
        
        if (bytesRead > 0 && spiRxBuffer[0] != 0x00)
        {
            lastCommand = spiRxBuffer[0];
            spiCommandReceived = true;
        }
    }
}

// En main():
void main(){
    // ...código...

    // Inicializar SPI Slave
    SERCOM2_SPI_Initialize();
    SERCOM2_SPI_CallbackRegister(SPI_SlaveCallback, 0);
    
    // Inicializar sistema BLE slave
    BLE_slave_init();
    
    // ...más...
}
```

---

## 7. Procesamiento de Comandos SPI

Además, se incluyen los archivos [ble_slave.c](../src/ble_slave.c) y [AVM_ble_cmd.c](../src/AVM_ble_cmd.c) para procesar los comandos como en el AVM.

El main los llama así:

```c
// Función para procesar comandos SPI recibidos
void ProcessSPICommands(void)
{
    if (spiCommandReceived)
    {
        // Usar el nuevo sistema BLE para procesar comandos
        BLE_processCommand(lastCommand, spiRxBuffer, lastBytesReceived);
        
        // Resetear flag
        spiCommandReceived = false;
    }
}
```

---

[⬅️ Volver al README](./../README.md)
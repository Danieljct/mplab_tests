# Configuración de I2C (SERCOM)

## 1. Agregar el Periférico SERCOM

En la ventana **Device Resources**, busca el SERCOM correspondiente:

```
Libraries → Harmony → Peripherals → SERCOMx
```

> ℹ️ **Nota:**  
> Para saber qué SERCOM corresponde a los pines físicos, consulta el capítulo 6 del [datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/DataSheets/SAM-D5x-E5x-Family-Data-Sheet-DS60001507.pdf).

---

## 2. Configuración del SERCOM como I2C

Una vez agregado el SERCOM, haz clic sobre él para abrir las **Configuration Options**.  
En el combobox **Select SERCOM operation mode**, selecciona **I2C Master** (no probado en modo slave).

<div align="center">
	<img src="images/i2c_config.png" alt="I2C config" width="500"/>
</div>

> ⚙️ Las configuraciones mostradas corresponden a valores estándar y pueden variar según el dispositivo esclavo.

---

## 3. Configuración de Pines

En el plugin **Pin Configurator**, busca los pines físicos y asígnales el SERCOM y su pad correspondiente:

<div align="center">
	<img src="images/i2c_pines.png" alt="I2C pines" width="900"/>
</div>

---

## 4. Configurar reloj

En el plugin **Clock configurator**, busca la sección de Peripheral Clock Configurator, baja hasta llegar al SERCOM correspondiente y asegúrate que tenga reloj asignado.

<div align="center">
	<img src="images/sercom_reloj.png" alt="SERCOM CLK" width="600"/>
</div>




## 4.5 (Opcional) Agregar Driver

> ⚠️ **Advertencia:**  
> Se puede agregar el driver desde el *Project Graph*. En algunos casos funciona correctamente, pero en otros puede presentar problemas.

<div align="center">
	<img src="images/i2c_drv.png" alt="I2C driver" width="400"/>
</div>

---

## 5. Generación de Código y Funciones Disponibles

Al generar el código, se agregan las librerías de I2C en:

```
src/config/default/peripheral/sercom/i2c_master/
```

**Funciones principales:**

```c
void SERCOM1_I2C_Initialize(void);

bool SERCOM1_I2C_Read(uint16_t address, uint8_t* rdData, uint32_t rdLength);

bool SERCOM1_I2C_Write(uint16_t address, uint8_t* wrData, uint32_t wrLength);

bool SERCOM1_I2C_WriteRead(uint16_t address, uint8_t* wrData, uint32_t wrLength, uint8_t* rdData, uint32_t rdLength);

bool SERCOM1_I2C_IsBusy(void);
```

> 📄 El archivo [codec.c](./../src/codec.c) contiene un ejemplo de uso del I2C.

---

[⬅️ Volver al README](./../README.md)
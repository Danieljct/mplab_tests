
# 🛠️ Tutorial: Cómo usar MPLAB

## Recursos oficiales
🔗 [Datasheet SAM D5x/E5x Family](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/DataSheets/SAM-D5x-E5x-Family-Data-Sheet-DS60001507.pdf)

🔗 [Tutoriales oficiales de Microchip Technology](https://mu.microchip.com/page/embedded-system-design)

🔗 [Periféricos para SAM y PIC32 (Deep Dive)](https://mu.microchip.com/sam-and-pic32-peripheral-deep-dive)

---

## 🚀 Creación de un nuevo proyecto

Recomiendo seguir este tutorial como primer paso:

🔗 [Crear tu primer proyecto Harmony 3](https://microchip-mplab-harmony.github.io/quick_docs/source/basic/create_first_harmony_3_project/readme.html)

### Videos recomendados para ATSAM

- [Introducción a Harmony 3 (YouTube)](https://www.youtube.com/watch?v=mL8Ge7LzGkA)
- [Configuración básica de Harmony 3 (YouTube)](https://www.youtube.com/watch?v=9MvtTYReosY)

---

## ⚙️ Configuración de periféricos AVM

### ⏰ Configuración de relojes

Al abrir el plugin **Clock Configurator** en el MCC:

<div align="center">
	<img src="assets/images/clock_plugin.png" alt="Clock configurator" width="350"/>
</div>

Se abre una interfaz como la siguiente:

<div align="center">
	<img src="assets/images/clock_configurator.png" alt="Clock configurator2" width="600"/>
</div>

De esta ventana destaca la sección **GCLK Generator x**, que permite crear hasta 10 relojes independientes entre sí, pero solo siendo un divisor entero de los de origen. En la sección **Peripheral Clock** se debe asignar a cada periférico un reloj de los generados.

> ⚠️ **Nota:** Intenté crear un FDPLL 1 pero el MCU no arrancaba después de eso.

---

### 🖲️ Configuración de pines

Abre el plugin **Pin Configuration** en el MCC, como se muestra:

<div align="center">
	<img src="assets/images/pin_config.png" alt="Pin Configuration Plugin" width="350"/>
</div>

Se recomienda, en primera instancia, cambiar el paquete:

<div align="center">
	<img src="assets/images/package.png" alt="Cambiar paquete" width="350"/>
</div>

---

## 📚 Periféricos utilizados en el AVM

- [Configuración de GPIO](./assets/GPIO.md)
- [Configuración de Timers](./assets/Timers.md)
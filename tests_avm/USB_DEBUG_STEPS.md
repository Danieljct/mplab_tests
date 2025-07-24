# USB Debug Steps - Código 43 Persistente

## Cambios Realizados Sin Éxito:
1. ✅ Corregidos descriptores USB malformados
2. ✅ Cambiado VID/PID (Arduino, FTDI)
3. ✅ Eliminados mensajes automáticos durante enumeración
4. ✅ Cambiado USB 2.0 → USB 1.1
5. ✅ Cambiado clase device de CDC → Generic
6. ✅ Reducido consumo de energía (100mA → 50mA)
7. ✅ Cambiado a Bus-Powered
8. ✅ Simplificada aplicación USB

## Próximos Pasos Recomendados:

### A. Verificación de Hardware (CRÍTICO):
```
1. Verificar soldadura/conexiones D+/D- en PCB
2. Probar con cable USB diferente (preferir cable corto)
3. Verificar alimentación 3.3V estable
4. Revisar cristal/oscilador USB (48MHz)
5. Verificar resistencias pull-up en D+ (si es High-Speed)
```

### B. Test con USB Analyzer:
```
1. Usar USBPcap + Wireshark para capturar tráfico
2. Ver en qué punto exacto falla la enumeración
3. Revisar descriptor requests vs responses
```

### C. Configuración de MPLAB Harmony (MCC):
```
1. Abrir archivo .mc3 en MPLAB Harmony Configurator
2. Verificar configuración USB Device Driver
3. Regenerar configuración desde cero
4. Verificar clock configuration (USB needs 48MHz)
```

### D. Test Alternativo - Crear HID Simple:
```
1. Cambiar temporalmente a USB HID (más simple que CDC)
2. Si HID funciona → problema en CDC stack
3. Si HID falla → problema de hardware/clocks
```

### E. Verificación de Clocks:
```
El USB requiere clock de 48MHz exacto:
- Verificar DFLL48M configuration
- Verificar que GCLK_USB esté configurado correctamente
- Clock drift puede causar enumeración fallida
```

## Código 43 - Causas Restantes Posibles:
- **Hardware**: D+/D- mal conectados, crystal mal, power dirty
- **Clocks**: 48MHz no exacto o no estable  
- **Timing**: Device responde muy lento/rápido
- **Harmony Config**: Algo mal en configuración MCC

## Comando de Test Hardware:
Si nada funciona, test con LED en main loop para verificar que el código básico corre:

```c
int main(void) {
    SYS_Initialize(NULL);
    while(1) {
        LED_R_Toggle();
        _delay_ms(500);
        SYS_Tasks();  // Comentar USB temporalmente
    }
}
```

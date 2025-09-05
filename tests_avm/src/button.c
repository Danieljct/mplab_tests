#include "definitions.h"
#include "button.h"
#include "config/default/interrupts.h"

// Inicialización del pin PA5 como entrada con interrupción externa
void BTN_InterruptInit(void)
{
    // Habilita la detección de flanco de bajada en EXTINT5
    EIC_REGS->EIC_CONFIG[0] = (EIC_REGS->EIC_CONFIG[0] & ~(0x7 << (5 * 4))) | (0x2 << (5 * 4)); // SENSE5 = 0x2 (falling edge)
    EIC_REGS->EIC_INTFLAG = (1 << 5); // Limpia bandera
    EIC_REGS->EIC_INTENSET = (1 << 5); // Habilita interrupción en línea 5

    // Habilita la detección de flanco de bajada en EXTINT6
    EIC_REGS->EIC_CONFIG[0] = (EIC_REGS->EIC_CONFIG[0] & ~(0x7 << (6 * 4))) | (0x2 << (6 * 4)); // SENSE6 = 0x2 (falling edge)
    EIC_REGS->EIC_INTFLAG = (1 << 6); // Limpia bandera
    EIC_REGS->EIC_INTENSET = (1 << 6); // Habilita interrupción en línea 6
    EIC_REGS->EIC_CTRLA |= EIC_CTRLA_CKSEL_Msk;  // Usa el reloj CLK_ULP32K para el EIC
    EIC_REGS->EIC_CTRLA |= EIC_CTRLA_ENABLE_Msk; // Habilita el EIC
}

// Handler de la interrupción externa 5 (PA5/BTN_1)
void EIC_EXTINT_5_Handler(void)
{
    // Limpia la bandera de interrupción
    EIC_REGS->EIC_INTFLAG = (1 << 5);
    LED_B_Toggle();
    SYS_CONSOLE_PRINT("Botón presionado!\r\n");
    // Acción: alternar el LED_B
    
}

// Handler de la interrupción externa 6 (PA6/BTN_2)
void EIC_EXTINT_6_Handler(void)
{
    // Limpia la bandera de interrupción
    EIC_REGS->EIC_INTFLAG = (1 << 6);
    LED_G_Toggle();
    SYS_CONSOLE_PRINT("Botón 2 presionado!\r\n");
}
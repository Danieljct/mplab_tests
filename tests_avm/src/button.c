#include "definitions.h"
#include "button.h"
#include "config/default/interrupts.h"

// Configuración de pines
#define BTN_1_PIN    PORT_PIN_PA05
#define BTN_2_PIN    PORT_PIN_PA06

// CORREGIDO: Tiempos ajustados para timer de 50ms
#define TIMEOUT_300MS_TICKS    6    // 300ms / 50ms = 6 ticks
#define TIMEOUT_30MS_TICKS     1    // 30ms / 50ms = 0.6 ≈ 1 tick  
#define LONG_CHECK_TICKS       1    // 50ms / 50ms = 1 tick
#define LONG_PRESS_THRESHOLD   20   // 50 * 50ms = 2.5 segundos

typedef enum {
    TIMER_STATE_IDLE = 0,
    TIMER_STATE_TIMEOUT_300MS,    // Timer de 300ms para detectar simple/doble
    TIMER_STATE_TIMEOUT_30MS,     // Timer corto de 30ms 
    TIMER_STATE_LONG_CHECK        // Timer de 50ms para verificar toque largo
} timer_state_t;

// Variables internas
static volatile button_gesture_t gesto_marcado = NO_PRESS;
static volatile bool flag_button_press = false;
static volatile bool timer_active = false;
static volatile timer_state_t timer_mode = TIMER_STATE_IDLE;
static volatile uint8_t btn_1_count = 0;
static volatile uint8_t btn_2_count = 0;
static volatile uint16_t timer_counter = 0;

// Variables para debug
static volatile uint32_t debug_timer_ticks = 0;
static volatile uint32_t debug_btn1_presses = 0;
static volatile uint32_t debug_btn2_presses = 0;

// Función equivalente a TIMER_0_task2_cb (timeout principal)
static void BTN_ProcessMainTimeout(void)
{
    timer_active = false;
    timer_mode = TIMER_STATE_IDLE;
    uint8_t suma = btn_1_count + btn_2_count;
    gesto_marcado = NO_PRESS;
    flag_button_press = true;
    
    switch(suma) {
        case 0: {
            // No hay botones presionados
            gesto_marcado = NO_PRESS;
            flag_button_press = false;
            break;
        }
        
        case 1: {
            // Verificar si algún botón sigue presionado para toque largo
            if (!BTN_1_Get() || !BTN_2_Get()) {
                // Botón sigue presionado, preparar para toque largo
                if (!BTN_1_Get()) btn_1_count += 10;
                if (!BTN_2_Get()) btn_2_count += 10;
                flag_button_press = false;
                
                // Iniciar timer de verificación larga
                timer_counter = 0;
                timer_mode = TIMER_STATE_LONG_CHECK;
                timer_active = true;
    
                break;
            } else {
                // Botón soltado - es toque simple
                if (btn_1_count >= 1) {
                    gesto_marcado = SIMPLE1;
    
                }
                if (btn_2_count >= 1) {
                    gesto_marcado = SIMPLE2;
            
                }
                break;
            }
        }
        
        case 2: {
            // Doble toque
            if (btn_1_count == 2) {
                gesto_marcado = DOBLE1;
          
            }
            if (btn_2_count == 2) {
                gesto_marcado = DOBLE2;
             
            }
            break;
        }
        
        case 3: {
            // Triple toque o combinación
            if (btn_1_count == 3 && btn_2_count == 0) {
                // Triple toque en botón 1 - tratar como doble
                gesto_marcado = DOBLE1;
           } else if (btn_2_count == 3 && btn_1_count == 0) {
                // Triple toque en botón 2 - tratar como doble  
                gesto_marcado = DOBLE2;
            } else if (btn_1_count > 0 && btn_2_count > 0) {
                // Combinación de botones
                gesto_marcado = DOSBTN;
            }
            break;
        }
        
        default: {
            // Más de 3 toques - decidir qué hacer
            if (btn_1_count > 0 && btn_2_count > 0) {
                gesto_marcado = DOSBTN;
            } else if (btn_1_count >= 2) {
                gesto_marcado = DOBLE1;
            } else if (btn_2_count >= 2) {
                gesto_marcado = DOBLE2;
            } else {
                gesto_marcado = NO_PRESS;
            }
            break;
        }
    }

    // Reset contadores solo si no vamos a verificar toque largo
    if (timer_mode != TIMER_STATE_LONG_CHECK) {
        btn_1_count = 0;
        btn_2_count = 0;
    }
}

// Función equivalente a TIMER_0_task_cb (verificación de toque largo)
static void BTN_ProcessLongCheck(void)
{
    
    // Verificar si ya es toque largo
    if (btn_1_count >= LONG_PRESS_THRESHOLD || btn_2_count >= LONG_PRESS_THRESHOLD) {
        if (btn_1_count >= LONG_PRESS_THRESHOLD) {
            gesto_marcado = LONG1;
        }
        if (btn_2_count >= LONG_PRESS_THRESHOLD) {
            gesto_marcado = LONG2;
        }
        
        flag_button_press = true;
        btn_1_count = 0;
        btn_2_count = 0;
        timer_active = false;
        timer_mode = TIMER_STATE_IDLE;
    } else {
        // Verificar si botones siguen presionados
        if (!BTN_1_Get() || !BTN_2_Get()) {
            // Incrementar contadores mientras estén presionados
            if (!BTN_1_Get()) btn_1_count += 1;
            if (!BTN_2_Get()) btn_2_count += 1;
            
            // Continuar con timer de verificación larga
            timer_counter = 0;
            timer_mode = TIMER_STATE_LONG_CHECK;
            timer_active = true;
        } else {
            // Botones soltados antes de llegar a largo
            btn_1_count = 0;
            btn_2_count = 0;
            timer_active = false;
            timer_mode = TIMER_STATE_IDLE;
        }
    }
}

// CORREGIDO: Timer tick cada 50ms
void BTN_TimerTick_50ms(void)
{
    debug_timer_ticks++;
    

    
    if (timer_active) {
        timer_counter++;
        
        switch(timer_mode) {
            case TIMER_STATE_TIMEOUT_300MS:
                if (timer_counter >= TIMEOUT_300MS_TICKS) {
                    BTN_ProcessMainTimeout();
                }
                break;
                
            case TIMER_STATE_TIMEOUT_30MS:
                if (timer_counter >= TIMEOUT_30MS_TICKS) {
                    BTN_ProcessMainTimeout();
                }
                break;
                
            case TIMER_STATE_LONG_CHECK:
                if (timer_counter >= LONG_CHECK_TICKS) {
                    BTN_ProcessLongCheck();
                }
                break;
                
            default:
                timer_active = false;
                timer_mode = TIMER_STATE_IDLE;
                break;
        }
    }
}



// Equivalente a TIMER_0_short (300ms timeout)
static void BTN_StartTimeout300(void)
{
    timer_counter = 0;
    timer_mode = TIMER_STATE_TIMEOUT_300MS;
    timer_active = true;
}

// Equivalente a TIMER_0_short2 (30ms timeout)
static void BTN_StartTimeout30(void)
{
    timer_counter = 0;
    timer_mode = TIMER_STATE_TIMEOUT_30MS;
    timer_active = true;
}

// Función equivalente a button_pressed del código original
static void BTN_ButtonPressed(void)
{
    // Incrementar contadores según botones presionados
    if (!BTN_1_Get()) {
        btn_1_count += 1;
        debug_btn1_presses++;
    }
    
    if (!BTN_2_Get()) {
        btn_2_count += 1;
        debug_btn2_presses++;
    }
    
    // Verificar ambos botones presionados simultaneamente
    if (!BTN_1_Get() && !BTN_2_Get()) {
        gesto_marcado = DOSBTN;
        flag_button_press = true;
        btn_1_count = 0;
        btn_2_count = 0;
        timer_active = false;
        timer_mode = TIMER_STATE_IDLE;
        return;
    }
    
    // Lógica de timers como en el original
    if (timer_active && (timer_mode == TIMER_STATE_TIMEOUT_300MS || timer_mode == TIMER_STATE_TIMEOUT_30MS)) {
        // Si ya hay un timer de timeout activo, cambiarlo a 30ms
        BTN_StartTimeout30();
    } else {
        // Iniciar timer de 300ms
        BTN_StartTimeout300();
    }
}

void BTN_InterruptInit(void)
{
    // Configurar pines con interrupción por flanco de bajada solamente
    EIC_REGS->EIC_CONFIG[0] = (EIC_REGS->EIC_CONFIG[0] & ~(0x7 << (5 * 4))) | (0x2 << (5 * 4)); // SENSE5 = falling edge
    EIC_REGS->EIC_CONFIG[0] = (EIC_REGS->EIC_CONFIG[0] & ~(0x7 << (6 * 4))) | (0x2 << (6 * 4)); // SENSE6 = falling edge
    EIC_REGS->EIC_INTFLAG = (1 << 5) | (1 << 6); // Limpia banderas
    EIC_REGS->EIC_INTENSET = (1 << 5) | (1 << 6); // Habilita interrupciones
    EIC_REGS->EIC_CTRLA |= EIC_CTRLA_CKSEL_Msk;  // Usa el reloj CLK_ULP32K para el EIC
    EIC_REGS->EIC_CTRLA |= EIC_CTRLA_ENABLE_Msk; // Habilita el EIC


}

// Handler de la interrupción externa 5 (BTN_1)
void EIC_EXTINT_5_Handler(void)
{
    EIC_REGS->EIC_INTFLAG = (1 << 5);
    BTN_ButtonPressed();
}

// Handler de la interrupción externa 6 (BTN_2)
void EIC_EXTINT_6_Handler(void)
{
    EIC_REGS->EIC_INTFLAG = (1 << 6);
    BTN_ButtonPressed();
}

// Devuelve el gesto detectado y limpia el flag
button_gesture_t BTN_getGesture(void)
{
    if (flag_button_press) {
        flag_button_press = false;
        button_gesture_t ret = gesto_marcado;
        gesto_marcado = NO_PRESS;
        return ret;
    }
    return NO_PRESS;
}

// Devuelve true si hay un gesto pendiente de leer
bool BTN_getFlagStatus(void)
{
    return flag_button_press;
}

// Funciones de debug
uint32_t BTN_GetDebugBtn1Count(void) { return debug_btn1_presses; }
uint32_t BTN_GetDebugBtn2Count(void) { return debug_btn2_presses; }
uint32_t BTN_GetDebugTimerTicks(void) { return debug_timer_ticks; }
uint16_t BTN_GetBtn1Count(void) { return btn_1_count; }
uint16_t BTN_GetBtn2Count(void) { return btn_2_count; }
bool BTN_GetTimerActive(void) { return timer_active; }
uint8_t BTN_GetTimerMode(void) { return (uint8_t)timer_mode; }
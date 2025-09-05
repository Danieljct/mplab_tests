#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

// Enum de gestos básicos para dos botones
typedef enum {
    NO_PRESS = 0,
    SIMPLE1,
    SIMPLE2,
    DOBLE1,
    DOBLE2,
    DOSBTN,
    LONG1,
    LONG2,
    INVALID
} button_gesture_t;

// Inicializa las interrupciones de ambos botones
void BTN_InterruptInit(void);

// Llama esto en main loop para obtener el gesto detectado (y limpiar el flag)
button_gesture_t BTN_getGesture(void);

// Devuelve true si hay un gesto pendiente de leer
bool BTN_getFlagStatus(void);

void BTN_timer_tick(void);
void BTN_LongPressTick(void);


#endif


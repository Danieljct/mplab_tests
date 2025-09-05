#ifndef BUTTON_H
#define BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif // BUTTON_H
      @Parameters
        This function does not take any parameters.

      @Returns
        This function does not return any value.

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.
     */
    void BTN_1_InterruptInit(void);
    void BTN_2_InterruptInit(void);
    void EIC_EXTINT_5_Handler(void);
    void EIC_EXTINT_6_Handler(void);



#endif /* _BUTTON_H */

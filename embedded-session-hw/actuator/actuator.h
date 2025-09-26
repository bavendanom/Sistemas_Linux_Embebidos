#ifndef ACTUATOR_H
#define ACTUATOR_H

/* actuator.h
 * Interfaz genérica para actuadores con polimorfismo en C.
 */

#include <stdbool.h>

/* Declaración de la estructura "actuator" con punteros a función. */
typedef struct actuator {
    void *params;   /* Puntero genérico a configuración específica del actuador */

    /* Punteros a función que definen el "comportamiento polimórfico" */
    void (*activate)(struct actuator *self);
    void (*deactivate)(struct actuator *self);
    bool (*status)(struct actuator *self);
} actuator_t;

#endif /* ACTUATOR_H */

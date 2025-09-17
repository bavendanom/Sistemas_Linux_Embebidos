#ifndef ACTUATOR_H
#define ACTUATOR_H

// Estructura para un actuador genérico
typedef struct {
    void *params;  // Parámetros adicionales para cada tipo de actuador
    void (*activate)(void *params);  // Puntero a función para activar el actuador
    void (*deactivate)(void *params); // Puntero a función para desactivar el actuador
    int (*status)(void *params);  // Puntero a función para obtener el estado del actuador
} actuator_t;

// Declaración de funciones para los actuadores
void led_activate(void *params);
void led_deactivate(void *params);
int led_status(void *params);

void buzzer_activate(void *params);
void buzzer_deactivate(void *params);
int buzzer_status(void *params);

#endif // ACTUATOR_H

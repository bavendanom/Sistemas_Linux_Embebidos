#include "actuator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Configuración específica para un LED */
typedef struct {
    bool is_on;
    int pin;
} led_params_t;

/* Funciones específicas del LED */
static void led_activate(actuator_t *self) {
    led_params_t *cfg = (led_params_t *)self->params;
    cfg->is_on = true;
    printf("[LED] Activado en pin %d\n", cfg->pin);
}

static void led_deactivate(actuator_t *self) {
    led_params_t *cfg = (led_params_t *)self->params;
    cfg->is_on = false;
    printf("[LED] Desactivado en pin %d\n", cfg->pin);
}

static bool led_status(actuator_t *self) {
    led_params_t *cfg = (led_params_t *)self->params;
    return cfg->is_on;
}

/* Función de fábrica para crear un actuador LED */
actuator_t *create_led_actuator(int pin) {
    actuator_t *act = malloc(sizeof(actuator_t));
    if (!act) return NULL;

    led_params_t *cfg = malloc(sizeof(led_params_t));
    if (!cfg) {
        free(act);
        return NULL;
    }

    cfg->is_on = false;
    cfg->pin = pin;

    act->params = cfg;
    act->activate = led_activate;
    act->deactivate = led_deactivate;
    act->status = led_status;

    return act;
}

/* Función de destrucción */
void destroy_led_actuator(actuator_t *act) {
    if (act) {
        free(act->params);
        free(act);
    }
}

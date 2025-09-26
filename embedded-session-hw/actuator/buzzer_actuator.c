#include "actuator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Configuración específica para un buzzer */
typedef struct {
    bool is_on;
    int frequency;
} buzzer_params_t;

/* Funciones específicas del buzzer */
static void buzzer_activate(actuator_t *self) {
    buzzer_params_t *cfg = (buzzer_params_t *)self->params;
    cfg->is_on = true;
    printf("[BUZZER] Activado con frecuencia %d Hz\n", cfg->frequency);
}

static void buzzer_deactivate(actuator_t *self) {
    buzzer_params_t *cfg = (buzzer_params_t *)self->params;
    cfg->is_on = false;
    printf("[BUZZER] Desactivado\n");
}

static bool buzzer_status(actuator_t *self) {
    buzzer_params_t *cfg = (buzzer_params_t *)self->params;
    return cfg->is_on;
}

/* Función de fábrica para crear un actuador buzzer */
actuator_t *create_buzzer_actuator(int frequency) {
    actuator_t *act = malloc(sizeof(actuator_t));
    if (!act) return NULL;

    buzzer_params_t *cfg = malloc(sizeof(buzzer_params_t));
    if (!cfg) {
        free(act);
        return NULL;
    }

    cfg->is_on = false;
    cfg->frequency = frequency;

    act->params = cfg;
    act->activate = buzzer_activate;
    act->deactivate = buzzer_deactivate;
    act->status = buzzer_status;

    return act;
}

/* Función de destrucción */
void destroy_buzzer_actuator(actuator_t *act) {
    if (act) {
        free(act->params);
        free(act);
    }
}

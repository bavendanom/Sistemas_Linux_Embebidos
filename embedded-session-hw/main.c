#include <stdio.h>
#include "sensor/sensor.h"
#include "actuator/actuator.h"

/* Declaraciones de las fábricas y destructores */
actuator_t *create_led_actuator(int pin);
void destroy_led_actuator(actuator_t *act);

actuator_t *create_buzzer_actuator(int frequency);
void destroy_buzzer_actuator(actuator_t *act);

int main(void) {
    /* Inicializar el sensor */
    sensor_init();

    /* Crear actuadores */
    actuator_t *led = create_led_actuator(13);
    actuator_t *buzzer = create_buzzer_actuator(2000);

    printf("=== Prueba unificada: Sensores + Actuadores ===\n");

    /* Simulación: tomar varias lecturas y actuar sobre actuadores */
    for (int i = 0; i < 10; ++i) {
        double lectura = sensor_read();
        printf("\nLectura %2d del sensor: %7.3f\n", i + 1, lectura);

        /* Lógica de control:
         * - Si temperatura > 30 → activar LED y buzzer
         * - Si temperatura entre 0 y 30 → solo LED
         * - Si temperatura < 0 → apagar ambos
         */
        if (lectura > 30.0) {
            led->activate(led);
            buzzer->activate(buzzer);
        } else if (lectura > 0.0) {
            led->activate(led);
            buzzer->deactivate(buzzer);
        } else {
            led->deactivate(led);
            buzzer->deactivate(buzzer);
        }

        printf("Estado LED: %s\n", led->status(led) ? "ON" : "OFF");
        printf("Estado BUZZER: %s\n", buzzer->status(buzzer) ? "ON" : "OFF");
    }

    /* Liberar memoria */
    destroy_led_actuator(led);
    destroy_buzzer_actuator(buzzer);

    return 0;
}


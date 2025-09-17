#include <stdio.h>
#include "actuator.h"

int main(void) {
    // Crear instancias de los actuadores LED y Zumbador
    actuator_t led = {NULL, led_activate, led_deactivate, led_status};
    actuator_t buzzer = {NULL, buzzer_activate, buzzer_deactivate, buzzer_status};

    // Activar y desactivar el LED
    printf("Operación con LED:\n");
    led.activate(led.params);   // Activar LED
    led.status(led.params);     // Verificar estado del LED
    led.deactivate(led.params); // Desactivar LED

    // Activar y desactivar el zumbador
    printf("\nOperación con Zumbador:\n");
    buzzer.activate(buzzer.params);   // Activar zumbador
    buzzer.status(buzzer.params);     // Verificar estado del zumbador
    buzzer.deactivate(buzzer.params); // Desactivar zumbador

    return 0;
}

#include "actuator.h"
#include <stdio.h>

void buzzer_activate(void *params) {
	(void)params;
    printf("Zumbador activado\n");
    // Aquí iría la lógica para activar el zumbador
}

void buzzer_deactivate(void *params) {
	(void)params;
    printf("Zumbador desactivado\n");
    // Aquí iría la lógica para desactivar el zumbador
}

int buzzer_status(void *params) {
	(void)params;
    // Aquí iría la lógica para verificar si el zumbador está activado o desactivado
    printf("Estado del zumbador: activado\n");
    return 1;  // Suponiendo que el zumbador está activado
}

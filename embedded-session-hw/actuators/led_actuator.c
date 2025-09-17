#include "actuator.h"
#include <stdio.h>

void led_activate(void *params) {
	(void)params;
    printf("LED activado\n");
}

void led_deactivate(void *params) {
	(void)params;
    printf("LED desactivado\n");
   
}

int led_status(void *params) {
	(void)params;
    printf("Estado del LED: encendido\n");
    return 1;  // Suponiendo que el LED está encendido
}

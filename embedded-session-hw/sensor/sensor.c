#include "sensor.h"
#include <stdlib.h>
#include <time.h>

// Inicializa el sensor (semilla para rand)
void sensor_init(void) {
    srand((unsigned int)time(NULL));  // Semilla aleatoria
}

// Devuelve un valor simulado entre 0.0 y 100.0
double sensor_read(void) {
    return (double)(rand() % 10000) / 100.0;  // Ej: 73.42
}

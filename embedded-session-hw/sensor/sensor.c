#include "sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Definición de la variable global
double sensor_value = 0.0;

// Función para inicializar el sensor
void sensor_init(void) {
    // Inicialización de cualquier hardware o variables necesarias
    srand(time(NULL)); // Inicializa el generador de números aleatorios
}

// Función para leer el valor del sensor
double sensor_read(void) {
    // Para simular el comportamiento del sensor, generamos un valor aleatorio
    // En un caso real, se leería el valor de un hardware específico o de un archivo CSV

    // Aquí usamos un valor aleatorio para simular la lectura del sensor
    sensor_value = (rand() % 1000) / 10.0; // Valor entre 0.0 y 100.0

    // También se puede leer un archivo CSV si se desea:
    // FILE *file = fopen("sensor_feed.csv", "r");
    // if (file != NULL) {
    //     fscanf(file, "%lf", &sensor_value);  // Leer un valor del archivo
    //     fclose(file);
    // }

    return sensor_value;
}

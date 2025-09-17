#ifndef SENSOR_H  // Include guard
#define SENSOR_H

// Declaración de las funciones
void sensor_init(void);  // Inicializa el sensor
double sensor_read(void); // Lee el valor del sensor

// Declaración de una variable global (extern) si es necesario
extern double sensor_value; // Variable global para almacenar el valor del sensor

#endif // SENSOR_H

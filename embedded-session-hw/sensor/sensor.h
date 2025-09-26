#ifndef SENSOR_H
#define SENSOR_H

/* sensor.h
 * Declaraciones públicas de la biblioteca de sensores.
 * Uso de include guard para evitar inclusiones múltiples.
 */

#include <stdint.h>

/* Ejemplo de variable global compartida entre módulos.
 * Declarada aquí con extern; definida en sensor.c.
 * El usuario puede sobrescribir sensor_seed antes de llamar sensor_init()
 * para obtener un patrón reproducible.
 */
extern int sensor_seed;

/* Inicializa el subsistema de sensor (si es necesario, siembra RNG, etc.) */
void sensor_init(void);

/* Lee el sensor y devuelve un valor en double.
 * Implementación en sensor.c. Para el ejercicio, devuelve valores simulados.
 */
double sensor_read(void);

#endif /* SENSOR_H */

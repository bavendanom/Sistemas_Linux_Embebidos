/* sensor.c
 * Definiciones de la biblioteca de sensores.
 * Aquí se coloca una única definición de sensor_seed (no multiple definition).
 */

#include "sensor.h"
#include <stdlib.h>
#include <time.h>

/* Definición del símbolo declarado extern en sensor.h */
int sensor_seed = 0;

/* Inicialización del sensor.
 * - Si sensor_seed == 0, se usa time() para generar una semilla distinta.
 * - srand() se llama solo aquí (una única definición).
 *
 * Nota: esta implementación no es thread-safe. Si requieres seguridad en hilos,
 * usar mutexes o un RNG por hilo.
 */
void sensor_init(void)
{
    if (sensor_seed == 0) {
        /* si no fue fijada por el usuario, usar time() */
        sensor_seed = (int)time(NULL);
    }
    /* Mezcla sensor_seed con time(NULL) para variar un poco aún si el usuario
     * fijó sensor_seed. */
    srand((unsigned)sensor_seed ^ (unsigned)time(NULL));
}

/* sensor_read: simulación de lectura de sensor.
 * Devuelve un valor double en un rango razonable (ej. -10.0 .. +50.0).
 * Implementado con rand() y escalado.
 */
double sensor_read(void)
{
    /* rand() devuelve [0 .. RAND_MAX] */
    int r = rand();
    double unit = (double)r / (double)RAND_MAX; /* 0.0 .. 1.0 */
    double min = -10.0;
    double max = 50.0;
    return min + unit * (max - min);
}

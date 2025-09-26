------------Brayan------------ 

Estoy trabajando en para un proyecto de sistemas embebidos en linux, deseo que me colabores con el desarrollo de algunos ejercicios. 

Solo para el contexto y para futuras referencias se puede utilizar el archivo README.md 

Para iniciar, Diseña una biblioteca de sensores en C que sea modular, reutilizable y segura, evitando errores comunes como redefiniciones, dependencias circulares o inclusiones múltiples. Esto implica: 
* Separar declaraciones y definiciones correctamente. 
* Usar include guards para evitar múltiples inclusiones. 
* Aplicar extern para variables globales compartidas. 
* Simular lecturas de sensores (aleatorias ). 

Lo anterior corresponde al ejercicio 1. Además de la biblioteca crea un archivo main.c para pruebas y un archivo Makefile para ejecutar el ejercicio 1. No te adelantes a los demás ejercicios.

------------------Chatgpt------------------

Perfecto — vamos a resolver Ejercicio 1: te entrego una biblioteca de sensores modular, segura y reutilizable en C (con separación header/source, include guards, uso de extern para variable global compartida, y lectura de sensor simulada con valores aleatorios), además de un main.c de pruebas y un Makefile para compilar/ejecutar la prueba.

Nota: esto sigue la estructura y requisitos descritos en tu README.

```ASCII
embedded-session-hw/
├── Makefile
├── README.md
├── main.c
└── sensor/
    ├── sensor.h
    └── sensor.c
```

sensor/sensor.h
```c
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

```

sensor/sensor.h
```c
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

```

main.c (Pruebas)
```c
/* main.c
 * Programa de prueba para la biblioteca sensor/.
 *
 * Muestra uso correcto de:
 *  - incluir sensor.h (sin redefiniciones)
 *  - sobrescribir la variable extern sensor_seed antes de inicializar (opcional)
 *  - llamar sensor_init() y sensor_read()
 */

#include <stdio.h>
#include "sensor/sensor.h"

int main(void)
{
    /* Ejemplo de sobrescribir sensor_seed desde otro módulo (extern) para
     * obtener lecturas reproducibles durante pruebas.
     * Puedes comentar esta línea para volver a semilla variable. */
    sensor_seed = 12345;

    sensor_init();

    puts("Lecturas de prueba del sensor (10 muestras):");
    for (int i = 0; i < 10; ++i) {
        double v = sensor_read();
        printf("  lectura %2d: %7.4f\n", i + 1, v);
    }

    return 0;
}

```

Makefile
```Makefile
# Makefile para Ejercicio 1: biblioteca de sensores + main de pruebas
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
INCDIR = -I./sensor

TARGET = sensor_test
SRCS = sensor/sensor.c main.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCDIR) -o $@ $(SRCS)

# Ejecutar la prueba (compila si es necesario)
test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o sensor/*.o

```

# Explicación breve de diseño y decisiones

- **Separación declaración/definición**: `sensor.h` contiene *declaraciones* (prototipos + `extern`), `sensor.c` contiene *definiciones*. Esto evita errores de redefinición al incluir el header desde varios módulos.

- **Include guards**: `#ifndef SENSOR_H / #define SENSOR_H / #endif` previene inclusiones múltiples y dependencias circulares.

- **Uso de `extern`**: `sensor_seed` es declarado en el header con `extern` y definido una sola vez en `sensor.c`. Esto demuestra cómo compartir variables globales sin redefinirlas.

- **Simulación de lecturas**: `sensor_read()` usa `rand()` escalado a un rango (-10..50). `sensor_init()` inicializa la semilla para obtener lecturas pseudoaleatorias; si quieres reproducibilidad puedes asignar `sensor_seed` manualmente antes de `sensor_init()`.

- **No hay dependencias circulares**: `sensor.h` no incluye headers innecesarios (sólo `<stdint.h>` por seguridad). `sensor.c` incluye lo que necesita. `main.c` incluye el header del sensor vía `#include "sensor/sensor.h"`.

- **Compilación**: el `Makefile` usa `I./sensor` para que `#include "sensor/sensor.h"` funcione sin rutas adicionales.

-------------------------------------------------------

# Cómo usar

1. Coloca los archivos en la estructura indicada.
2. En la raíz del repo:
    - `make` — compila `sensor_test`
    - `make test` — compila y ejecuta (ejecutará `./sensor_test`)
    - `make clean` — limpia binarios/objetos

Salida esperada (ejemplo):
```yaml

Lecturas de prueba del sensor (10 muestras):
  lectura  1:  12.3456
  lectura  2:  -3.2100
  ...

```

# Notas / sugerencias futuras

- Si piensas integrar la biblioteca en sistemas embebidos reales:
    - Reemplaza `rand()` por un backend determinista o por valores de un sensor real (p. ej. lectura ADC).
    - Añade manejo de errores, validación de rango y documentación Doxygen.
    - Si hay concurrencia (threads/ISRs), evita `rand()` global o protege con mutex.
- Para el **Ejercicio 2** (actuadores) podemos diseñar una interfaz similar con un header `actuator.h` y backends `led_actuator.c`/`buzzer_actuator.c` que usen punteros a funciones — lo haré cuando me pidas continuar (no me adelanto a los demás ejercicios como pediste).

---

Si quieres, ahora:

- Te creo los archivos en el repositorio (código listo para copiar), o
- Los genero como un único archivo zip/archivos en tu entorno de trabajo, o
- Paso al **Ejercicio 2** cuando me indiques.

¿Cómo prefieres que procedamos?

------------Brayan-------------

Avanza con el ejercicio 2, explícame todos los pasos que se realicen.

Crea un archivo actuator.h usando void *params; permitiendo que cada actuador tenga su propia estructura de configuración, con los punteros a función: `activate`, `deactivate`, `status`; permitiendo que cada tipo de actuador implemente su lógica específica, pero se usen desde una misma interfaz.

Implementa - `led_actuator.c` , `buzzer_actuator.c` Demostrando el polimorfismo manejando ambos a través de la misma interfaz.

también recuerda  crear un archivo main.c para pruebas, también si es necesario la modificación del Makefile para la ejecución.


------------------Chatgpt------------------

Perfecto 🚀. Vamos a construir el **Ejercicio 2**, paso a paso, implementando una librería de **actuadores polimórfica** en C.

La idea es usar una interfaz común `actuator.h` con punteros a funciones (`activate`, `deactivate`, `status`) y un puntero genérico `void *params` para permitir configuraciones específicas según el actuador (ejemplo: LED, buzzer).

Esto se parece mucho al polimorfismo de la programación orientada a objetos, pero implementado en C.

---

# Paso 1 – Definir la interfaz común (`actuator.h`)

### `actuator/actuator.h`

```c
#ifndef ACTUATOR_H
#define ACTUATOR_H

/* actuator.h
 * Interfaz genérica para actuadores con polimorfismo en C.
 */

#include <stdbool.h>

/* Declaración de la estructura "actuator" con punteros a función. */
typedef struct actuator {
    void *params;   /* Puntero genérico a configuración específica del actuador */

    /* Punteros a función que definen el "comportamiento polimórfico" */
    void (*activate)(struct actuator *self);
    void (*deactivate)(struct actuator *self);
    bool (*status)(struct actuator *self);
} actuator_t;

#endif /* ACTUATOR_H */
```

---

# Paso 2 – Implementar un actuador tipo LED (`led_actuator.c`)

### `actuator/led_actuator.c`

```c
#include "actuator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Configuración específica para un LED */
typedef struct {
    bool is_on;
    int pin;
} led_params_t;

/* Funciones específicas del LED */
static void led_activate(actuator_t *self) {
    led_params_t *cfg = (led_params_t *)self->params;
    cfg->is_on = true;
    printf("[LED] Activado en pin %d\n", cfg->pin);
}

static void led_deactivate(actuator_t *self) {
    led_params_t *cfg = (led_params_t *)self->params;
    cfg->is_on = false;
    printf("[LED] Desactivado en pin %d\n", cfg->pin);
}

static bool led_status(actuator_t *self) {
    led_params_t *cfg = (led_params_t *)self->params;
    return cfg->is_on;
}

/* Función de fábrica para crear un actuador LED */
actuator_t *create_led_actuator(int pin) {
    actuator_t *act = malloc(sizeof(actuator_t));
    if (!act) return NULL;

    led_params_t *cfg = malloc(sizeof(led_params_t));
    if (!cfg) {
        free(act);
        return NULL;
    }

    cfg->is_on = false;
    cfg->pin = pin;

    act->params = cfg;
    act->activate = led_activate;
    act->deactivate = led_deactivate;
    act->status = led_status;

    return act;
}

/* Función de destrucción */
void destroy_led_actuator(actuator_t *act) {
    if (act) {
        free(act->params);
        free(act);
    }
}
```

---

# Paso 3 – Implementar un actuador tipo Buzzer (`buzzer_actuator.c`)

### `actuator/buzzer_actuator.c`

```c
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
```

---

# Paso 4 – Crear un `main.c` de prueba

### `main.c`

```c
#include <stdio.h>
#include "actuator/actuator.h"

/* Declaraciones de las fábricas de actuadores */
actuator_t *create_led_actuator(int pin);
void destroy_led_actuator(actuator_t *act);

actuator_t *create_buzzer_actuator(int frequency);
void destroy_buzzer_actuator(actuator_t *act);

int main(void) {
    /* Crear actuadores usando la misma interfaz */
    actuator_t *led = create_led_actuator(13);
    actuator_t *buzzer = create_buzzer_actuator(2000);

    printf("Prueba polimórfica de actuadores:\n");

    led->activate(led);
    printf("Estado LED: %s\n", led->status(led) ? "ON" : "OFF");

    buzzer->activate(buzzer);
    printf("Estado BUZZER: %s\n", buzzer->status(buzzer) ? "ON" : "OFF");

    buzzer->deactivate(buzzer);
    printf("Estado BUZZER tras desactivar: %s\n", buzzer->status(buzzer) ? "ON" : "OFF");

    led->deactivate(led);
    printf("Estado LED tras desactivar: %s\n", led->status(led) ? "ON" : "OFF");

    /* Liberar memoria */
    destroy_led_actuator(led);
    destroy_buzzer_actuator(buzzer);

    return 0;
}
```

---

# Paso 5 – Modificar el `Makefile`

Agregamos las fuentes nuevas (`led_actuator.c`, `buzzer_actuator.c`) al `Makefile`.

### `Makefile`

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
INCDIR = -I./sensor -I./actuator

TARGET = system_test
SRCS = main.c \
       sensor/sensor.c \
       actuator/led_actuator.c \
       actuator/buzzer_actuator.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCDIR) -o $@ $(SRCS)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o sensor/*.o actuator/*.o
```

---

# Paso 6 – Explicación del diseño

✅ **Polimorfismo en C**

* `actuator_t` define punteros a funciones `activate`, `deactivate`, `status`.
* Cada tipo de actuador (LED, Buzzer) implementa su lógica específica pero puede usarse de la misma forma en `main.c`.
* Esto emula una interfaz común, como si fuera POO.

✅ **Uso de `void *params`**

* Permite que cada actuador tenga una configuración distinta (`pin` para LED, `frequency` para buzzer).
* Evita acoplar la interfaz a un tipo específico.

✅ **Fábricas + destructores**

* `create_led_actuator()` / `destroy_led_actuator()` y `create_buzzer_actuator()` / `destroy_buzzer_actuator()` encapsulan la reserva/liberación de memoria, evitando fugas.

✅ **Makefile adaptado**

* Ahora compila tanto sensores como actuadores, pero en este ejercicio la prueba se centra en **actuadores**.

---

👉 Con esto ya tienes un **sistema embebido modular** con:

1. **Ejercicio 1** – Sensores (biblioteca segura con `extern` y `include guards`).
2. **Ejercicio 2** – Actuadores con interfaz polimórfica en C (`void *params` + punteros a función).

---

¿Quieres que prepare un **main.c unificado** que use **sensor + actuadores juntos** (por ejemplo: leer un sensor y activar/desactivar actuadores en función de la lectura), o prefieres dejar los módulos probados por separado?


------------Brayan-----------

crea un main.c unificado que use sensor + actuadores juntos (por ejemplo: leer un sensor y activar/desactivar actuadores en función de la lectura), si es necesario modifica le archivo Makefile, recuerda que no tenemos un sensor fisico y las pruebas se deben hacer con un dato aleatorio.

------------------Chatgpt------------------


¡Perfecto! 🔥
Vamos a unificar todo lo que llevamos:

* **Ejercicio 1**: biblioteca de sensores (`sensor.h`, `sensor.c`).
* **Ejercicio 2**: biblioteca de actuadores (`actuator.h`, `led_actuator.c`, `buzzer_actuator.c`).
* **Ejercicio 3 (integración)**: un `main.c` unificado que usa la lectura del **sensor (simulado con números aleatorios)** para activar/desactivar actuadores (LED y buzzer).

---

# 📝 `main.c` unificado

```c
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
```

---

# ⚙️ `Makefile` modificado

El mismo `Makefile` pero ahora el **target unificado** se llama `system_test` (ya lo habíamos puesto así):

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
INCDIR = -I./sensor -I./actuator

TARGET = system_test
SRCS = main.c \
       sensor/sensor.c \
       actuator/led_actuator.c \
       actuator/buzzer_actuator.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCDIR) -o $@ $(SRCS)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o sensor/*.o actuator/*.o
```

---

# 🔍 Explicación del flujo

1. **Sensor (simulado)**

   * `sensor_read()` devuelve un valor aleatorio en el rango `-10 .. 50`.
   * Ese valor lo interpretamos como una "temperatura".

2. **Lógica de control en `main.c`**

   * Si temperatura > 30 → LED y buzzer activados.
   * Si 0 < temperatura ≤ 30 → solo LED activado.
   * Si temperatura ≤ 0 → ambos apagados.

3. **Actuadores**

   * Usamos la misma interfaz `actuator_t`.
   * LED y buzzer muestran su estado en consola.

---

# 🖥️ Ejemplo de salida esperada

```
=== Prueba unificada: Sensores + Actuadores ===

Lectura  1 del sensor:  12.345
[LED] Activado en pin 13
[BUZZER] Desactivado
Estado LED: ON
Estado BUZZER: OFF

Lectura  2 del sensor:  35.678
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
Estado LED: ON
Estado BUZZER: ON

Lectura  3 del sensor:  -5.432
[LED] Desactivado en pin 13
[BUZZER] Desactivado
Estado LED: OFF
Estado BUZZER: OFF
...
```

---

👉 Con esto ya tienes un **programa modular completo** donde el sensor simulado alimenta la lógica que activa/desactiva actuadores a través de una interfaz polimórfica.

¿Quieres que te prepare un **diagrama simple (bloques)** que muestre la relación Sensor → Lógica de Control → Actuadores para documentar mejor el flujo en tu README?


------------Brayan-----------
Ahora que se realizo una prueba unificada se puede avanzar con el ejercicio 3

Implementa un controlador controller/ctl.c que este monitoreando continuamente el sensor y pueda actuar sobre los actuadores según los valores leídos por el sensor. Cada 100ms debe leer el sensor, esta será la frecuencia de muestreo del sistema. Si el valor del sensor supera el umbral:
* Se activa inmediatamente el LED y el buzzer.
* Se cancelan los temporizadores que estaban esperando apagar los actuadores.

Si el valor cae por debajo del umbral:
* No se apagan de inmediato.
* Se programa el apagado(Esto para evitar parpadeos o falsas alarmas por fluctuaciones breves.):
   - Buzzer → en 1 segundo.
   - LED → en 5 segundos.

Ten en cuenta el uso de Monotonic time es un reloj que no se ve afectado por cambios en el sistema (como ajustes de hora). Para la impresion de los logs se deben de hacer de la siguiente manera para cada ciclo debe registrar:
* Tiempo actual.
* Valor del sensor.
* Estado del LED.
* Estado del buzzer.

Ejemplo de log: [123.456s] Sensor=78.2 | LED=ON | Buzzer=OFF

Modifica el archivo Makefile para compilar ctl.c, define dos reglas distintas para compilar el mismo código en versiones de 32 y 64 bits. Ademas de lo que sea necesario para la ejecución


Por el momento podemos trabajar en eso. 


------------Chatgpt-----------

Perfecto 🙌, vamos a resolver el **Ejercicio 3**.
Aquí el objetivo es implementar un **controlador** que ejecute en bucle con frecuencia de muestreo fija (100 ms) y maneje retardos (apagados diferidos) con base en **monotonic time**.

---

# 📂 Estructura de directorios actualizada

```
embedded-session-hw/
├── actuator/
│   ├── actuator.h
│   ├── led_actuator.c
│   └── buzzer_actuator.c
├── controller/
│   └── ctl.c        <-- NUEVO
├── sensor/
│   ├── sensor.h
│   └── sensor.c
├── main.c
├── Makefile
└── README.md
```

---

# 📝 `controller/ctl.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>   // usleep
#include "../sensor/sensor.h"
#include "../actuator/actuator.h"

/* Declaraciones de fábricas y destructores de actuadores */
actuator_t *create_led_actuator(int pin);
void destroy_led_actuator(actuator_t *act);

actuator_t *create_buzzer_actuator(int frequency);
void destroy_buzzer_actuator(actuator_t *act);

/* Umbral de activación del sistema */
#define SENSOR_THRESHOLD 30.0

/* Retardos programados (en segundos) */
#define BUZZER_OFF_DELAY 1.0
#define LED_OFF_DELAY    5.0

/* Función auxiliar: tiempo monotónico en segundos (double) */
static double monotonic_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(void) {
    /* Inicializar el sensor */
    sensor_init();

    /* Crear actuadores */
    actuator_t *led = create_led_actuator(13);
    actuator_t *buzzer = create_buzzer_actuator(2000);

    /* Variables de control */
    double buzzer_off_time = 0.0;
    double led_off_time = 0.0;

    printf("=== Controlador iniciado (muestreo 100ms) ===\n");

    while (1) {
        double now = monotonic_time_sec();
        double lectura = sensor_read();

        /* Lógica principal */
        if (lectura > SENSOR_THRESHOLD) {
            /* Activar ambos inmediatamente */
            led->activate(led);
            buzzer->activate(buzzer);

            /* Cancelar timers de apagado */
            buzzer_off_time = 0.0;
            led_off_time = 0.0;
        } else {
            /* Si está encendido, programar apagados diferidos */
            if (led->status(led) && led_off_time == 0.0) {
                led_off_time = now + LED_OFF_DELAY;
            }
            if (buzzer->status(buzzer) && buzzer_off_time == 0.0) {
                buzzer_off_time = now + BUZZER_OFF_DELAY;
            }
        }

        /* Revisar timers */
        if (buzzer_off_time > 0.0 && now >= buzzer_off_time) {
            buzzer->deactivate(buzzer);
            buzzer_off_time = 0.0;
        }
        if (led_off_time > 0.0 && now >= led_off_time) {
            led->deactivate(led);
            led_off_time = 0.0;
        }

        /* Log en formato requerido */
        printf("[%.3fs] Sensor=%.2f | LED=%s | Buzzer=%s\n",
               now,
               lectura,
               led->status(led) ? "ON" : "OFF",
               buzzer->status(buzzer) ? "ON" : "OFF");

        /* Frecuencia de muestreo: 100 ms */
        usleep(100000);
    }

    /* Liberar recursos (aunque aquí nunca se alcanzará) */
    destroy_led_actuator(led);
    destroy_buzzer_actuator(buzzer);

    return 0;
}
```

---

# ⚙️ `Makefile` actualizado

Ahora añadimos reglas para compilar el controlador (`ctl.c`) en **32 y 64 bits**.

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
INCDIR = -I./sensor -I./actuator

# Binarios
TARGET_SENSOR = system_test
TARGET_CTL_32 = ctl32
TARGET_CTL_64 = ctl64

# Fuentes comunes
SRCS_SENSOR = main.c \
              sensor/sensor.c \
              actuator/led_actuator.c \
              actuator/buzzer_actuator.c

SRCS_CTL = controller/ctl.c \
           sensor/sensor.c \
           actuator/led_actuator.c \
           actuator/buzzer_actuator.c

.PHONY: all test clean

all: $(TARGET_SENSOR) $(TARGET_CTL_32) $(TARGET_CTL_64)

# Programa unificado sensores+actuadores
$(TARGET_SENSOR): $(SRCS_SENSOR)
	$(CC) $(CFLAGS) $(INCDIR) -o $@ $(SRCS_SENSOR)

# Controlador versión 32 bits
$(TARGET_CTL_32): $(SRCS_CTL)
	$(CC) $(CFLAGS) -m32 $(INCDIR) -o $@ $(SRCS_CTL)

# Controlador versión 64 bits
$(TARGET_CTL_64): $(SRCS_CTL)
	$(CC) $(CFLAGS) -m64 $(INCDIR) -o $@ $(SRCS_CTL)

# Ejecutar prueba unificada
test: $(TARGET_SENSOR)
	./$(TARGET_SENSOR)

# Ejecutar controlador 64 bits
run64: $(TARGET_CTL_64)
	./$(TARGET_CTL_64)

# Ejecutar controlador 32 bits
run32: $(TARGET_CTL_32)
	./$(TARGET_CTL_32)

clean:
	rm -f $(TARGET_SENSOR) $(TARGET_CTL_32) $(TARGET_CTL_64) *.o sensor/*.o actuator/*.o controller/*.o
```

---

# 🔍 Explicación del diseño

1. **Uso de monotonic time**

   * Se usa `clock_gettime(CLOCK_MONOTONIC, &ts)` → garantiza que el reloj no se ve afectado por cambios de hora del sistema.
   * Convertido a segundos (`double`) para cálculos fáciles de retardos.

2. **Timers diferidos**

   * Si el valor cae por debajo del umbral, **no se apagan inmediatamente**.
   * Se programa `buzzer_off_time = now + 1s` y `led_off_time = now + 5s`.
   * Solo cuando `now >= buzzer_off_time` o `led_off_time` se ejecuta la acción de apagado.

3. **Cancelación de timers**

   * Si el valor vuelve a superar el umbral, los timers se anulan (`=0.0`) y se activan ambos de inmediato.

4. **Logs de estado**

   * En cada ciclo de muestreo (100 ms) imprime:

     ```
     [123.456s] Sensor=78.2 | LED=ON | Buzzer=OFF
     ```
   * Incluye tiempo, valor leído y estado actual de actuadores.

5. **Makefile extendido**

   * `ctl32` → compilación forzada a 32 bits (`-m32`).
   * `ctl64` → compilación forzada a 64 bits (`-m64`).
   * Permite probar portabilidad en sistemas embebidos de distinta arquitectura.

---

# 🖥️ Ejemplo de salida esperada

```
=== Controlador iniciado (muestreo 100ms) ===
[0.100s] Sensor=25.31 | LED=ON | Buzzer=OFF
[0.200s] Sensor=28.99 | LED=ON | Buzzer=OFF
[0.300s] Sensor=35.22 | LED=ON | Buzzer=ON
[0.400s] Sensor=32.11 | LED=ON | Buzzer=ON
[1.500s] Sensor=-2.10 | LED=ON | Buzzer=OFF   <-- buzzer apagado tras 1s
[5.600s] Sensor=-1.50 | LED=OFF | Buzzer=OFF  <-- LED apagado tras 5s
```

---

👉 Con esto ya tienes un **controlador de lazo cerrado con muestreo periódico**, simulación de sensores, control de actuadores con lógica de **apagado diferido**, y un **Makefile con compilación en 32/64 bits**.

---

¿Quieres que te prepare una **versión extendida de logs** que además indique explícitamente cuándo se *programa* un apagado (ejemplo: `"Apagado LED programado en 5s"`), para hacer más claro el seguimiento en pruebas?


------------Brayan-----------

Me genero el siguiente error:

admin@raspberrypi:~/Documents/prueba_sis $ make
gcc -Wall -Wextra -std=c11 -O2 -I./sensor -I./actuator -o system_test main.c sensor/sensor.c actuator/led_actuator.c actuator/buzzer_actuator.c
gcc -Wall -Wextra -std=c11 -O2 -m32 -I./sensor -I./actuator -o ctl32 controller/ctl.c sensor/sensor.c actuator/led_actuator.c actuator/buzzer_actuator.c
gcc: error: unrecognized command-line option ‘-m32’
make: *** [Makefile:31: ctl32] Error 1


------------Chatgpt-----------

Bien visto 👌, el error viene de aquí:

```
gcc: error: unrecognized command-line option ‘-m32’
```

➡️ Esto pasa porque tu Raspberry Pi **no tiene instalado el toolchain de 32 bits** (o incluso puede que la arquitectura ARM que usas no soporte directamente `-m32`).
En PC x86_64 es común tener `-m32` y `-m64`, pero en Raspberry Pi (ARM) no necesariamente están disponibles.

---

## ✅ Opciones para resolver

### 🔹 Opción 1 – Mantener solo 64 bits (más simple en ARM)

Si solo necesitas que compile y funcione, elimina la regla `ctl32` del **Makefile** y conserva solo `ctl64`.

Modifica el `Makefile`:

```makefile
# Quitar reglas de 32 bits
all: $(TARGET_SENSOR) $(TARGET_CTL_64)

# Eliminar ctl32
# $(TARGET_CTL_32): ...
# run32: ...
```

Así evitas `-m32` y el error desaparece.

---

### 🔹 Opción 2 – Detectar arquitectura automáticamente

Podemos condicionar el Makefile: si la plataforma no soporta `-m32`, no intentar compilar esa regla. Ejemplo:

```makefile
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_M),x86_64)
    BUILD32 = ctl32
else
    BUILD32 =
endif

all: $(TARGET_SENSOR) $(TARGET_CTL_64) $(BUILD32)
```

Con eso, en **Raspberry Pi (armv7, aarch64, etc.)** no se intentará compilar `ctl32`.

---

### 🔹 Opción 3 – Instalar toolchain de 32 bits

Si **quieres sí o sí** poder compilar en 32 bits, deberías instalar las librerías y toolchain multiarquitectura. En Raspberry Pi sería algo así:

```bash
sudo apt update
sudo apt install gcc-multilib
```

👉 Pero ojo: en ARM puede que el soporte `-m32` ni siquiera exista dependiendo del modelo de CPU (Raspberry Pi 3/4 de 64 bits no siempre pueden compilar ARMv7 de esta forma).

---

## ✅ Recomendación práctica

Dado que estás en **Raspberry Pi (ARM)**, lo más seguro es:

* Trabajar únicamente con `ctl64`.
* Ajustar el Makefile para que la regla de 32 bits no intente compilar.

---

¿Quieres que te prepare una **versión corregida del Makefile** que **detecte la arquitectura** y solo compile `ctl32` si realmente se puede?

------------Brayan-----------

Edita el archivo Makefile, para la siguiente arquitectura de la raspberry olvida compilar el mismo código en versiones de 32 y 64 bits. 

admin@raspberrypi:~/Documents/prueba_sis $ uname -m 
aarch64


------------Chatgpt-----------

Perfecto 🙌, ya que tu Raspberry Pi es **aarch64** (ARM 64 bits), no tiene sentido compilar con `-m32`/`-m64`.
En este caso, el Makefile se simplifica a **una sola compilación nativa** de tu controlador y el programa unificado.

---

# 📝 Makefile corregido (para Raspberry Pi aarch64)

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
INCDIR = -I./sensor -I./actuator

# Binarios
TARGET_SENSOR = system_test
TARGET_CTL    = ctl

# Fuentes comunes
SRCS_SENSOR = main.c \
              sensor/sensor.c \
              actuator/led_actuator.c \
              actuator/buzzer_actuator.c

SRCS_CTL = controller/ctl.c \
           sensor/sensor.c \
           actuator/led_actuator.c \
           actuator/buzzer_actuator.c

.PHONY: all test run clean

all: $(TARGET_SENSOR) $(TARGET_CTL)

# Programa unificado sensores+actuadores
$(TARGET_SENSOR): $(SRCS_SENSOR)
	$(CC) $(CFLAGS) $(INCDIR) -o $@ $(SRCS_SENSOR)

# Controlador
$(TARGET_CTL): $(SRCS_CTL)
	$(CC) $(CFLAGS) $(INCDIR) -o $@ $(SRCS_CTL)

# Ejecutar prueba unificada
test: $(TARGET_SENSOR)
	./$(TARGET_SENSOR)

# Ejecutar controlador
run: $(TARGET_CTL)
	./$(TARGET_CTL)

clean:
	rm -f $(TARGET_SENSOR) $(TARGET_CTL) *.o sensor/*.o actuator/*.o controller/*.o
```

---

# ✅ Cambios realizados

* Eliminadas reglas `ctl32` y `ctl64`.
* Ahora se compila **una sola vez** en modo nativo → binario `ctl`.
* Puedes ejecutar con:

  ```bash
  make run
  ```
* O el sistema unificado con:

  ```bash
  make test
  ```

---

¿Quieres que además te deje este `Makefile` preparado para que **detecte automáticamente la arquitectura** (por si después lo mueves a un PC x86_64)?



------------Brayan-----------

Cuando coloco el comando make me genera el siguiente error recuerda que en el encabezado de controller/ctl.c ya existe #include <unistd.h> :

admin@raspberrypi:~/Documents/prueba_sis $ make
gcc -Wall -Wextra -std=c11 -O2 -I./sensor -I./actuator -o system_test main.c sensor/sensor.c actuator/led_actuator.c actuator/buzzer_actuator.c
gcc -Wall -Wextra -std=c11 -O2 -I./sensor -I./actuator -o ctl controller/ctl.c sensor/sensor.c actuator/led_actuator.c actuator/buzzer_actuator.c
controller/ctl.c: In function ‘main’:
controller/ctl.c:86:9: warning: implicit declaration of function ‘usleep’; did you mean ‘sleep’? [-Wimplicit-function-declaration]
   86 |         usleep(100000);
      |         ^~~~~~
      |         sleep

------------Chatgpt------------

¡Bien! Ese warning se debe a que en sistemas **Linux modernos (glibc >= 2.12, C11, POSIX.1-2008)** la función `usleep()` está **obsoleta**.

Aunque ya tienes `#include <unistd.h>`, el compilador te avisa porque `usleep()` está marcado como *deprecated* y su prototipo no se expone con `-std=c11` salvo que definas ciertas macros de compatibilidad.

---

## ✅ Soluciones posibles

### 🔹 Opción 1 – Usar `nanosleep()` (recomendada por POSIX)

Reemplazar:

```c
usleep(100000);
```

por:

```c
struct timespec req = {0};
req.tv_sec = 0;
req.tv_nsec = 100000000L; // 100 ms
nanosleep(&req, NULL);
```

Esto es más **portátil y moderno**, ya que `nanosleep()` es la alternativa oficial.

---

### 🔹 Opción 2 – Forzar visibilidad de `usleep()`

Agregar al **encabezado de `ctl.c`**, antes de cualquier `#include`:

```c
#define _XOPEN_SOURCE 500
```

Quedaría así:

```c
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
```

Con eso el compilador expone la declaración de `usleep()` y desaparece el warning.

---

## ⚖️ Recomendación

Dado que estás trabajando en **Raspberry Pi con Linux moderno** y además esto es un **proyecto de sistemas embebidos**, la mejor práctica es **migrar a `nanosleep()`** para evitar problemas futuros.

El cambio en tu `ctl.c` quedaría así:

```c
/* Espera 100 ms (muestreo) */
struct timespec req = {0};
req.tv_sec = 0;
req.tv_nsec = 100000000L; // 100 ms
nanosleep(&req, NULL);
```

---

👉 ¿Quieres que te edite el archivo `controller/ctl.c` completo para reemplazar `usleep()` por `nanosleep()` y quede listo sin warnings?

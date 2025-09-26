
# Actuadores y un controlador con lazo de realimentación 🍓 


## Descripción del proyecto

Este un sistema embebido modular desarrollado en C para Linux (ejecutado en Raspberry Pi) que integra sensores simulados, actuadores polimórficos y un controlador en lazo cerrado.

El sistema se organiza en tres bloques principales:

* **Sensor** → Biblioteca que simula lecturas de un sensor (valores aleatorios) con inicialización segura y modularidad garantizada mediante *include guards* y manejo de variables globales con `extern`.
* **Actuadores** → Biblioteca polimórfica basada en punteros a función y `void *params`, que permite manejar distintos actuadores (LED, buzzer) a través de una misma interfaz común.
* **Controlador** → Módulo encargado de monitorear el sensor a una frecuencia de **100 ms** y decidir la activación/desactivación de los actuadores con retardos programados para evitar parpadeos o falsas alarmas.

  * LED → apagado diferido de **5 segundos**.
  * Buzzer → apagado diferido de **1 segundo**.
  * Ambos se activan inmediatamente al superar un umbral de lectura.

Este proyecto demuestra el diseño de un sistema embebido **modular, reutilizable y seguro**, aplicando buenas prácticas de programación en C y pensado para su uso didáctico en entornos de control en tiempo real.

## Tabla de contenidos
- [Requisitos](#requisitos)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Build](#build)
- [Ejemplo de ejecución](#ejemplo-de-ejecución)
- [Inspeccion binaria](#inspeccion-binaria)
- [Reflexión sobre errores](#reflexion-sobre-errores)

- [Autor](#autor)

## Requisitos
- Linux (probado en Raspberry Pi OS / Debian)
- `build-essential`
- `libncurses5-dev`
- `make`

## Estructura del proyecto

```ASCII
.
├── actuator
│   ├── actuator.h         # Interfaz común para actuadores (punteros a función + void *params).
│   ├── buzzer_actuator.c  # Implementación del actuador buzzer con retardo programado.
│   └── led_actuator.c     # Implementación del actuador LED con control ON/OFF.
├── ai_log.md              # Registro de avances, decisiones y explicaciones generadas con ayuda de IA.
├── controller
│   └── ctl.c              # Controlador principal: monitorea sensor cada 100ms y gestiona actuadores.
├── ctl                    # Binario compilado del controlador (ejecutable principal).
├── main.c                 # Programa de pruebas unificadas: integra sensor y actuadores con lógica básica.
├── Makefile               # Script de compilación: genera binarios (system_test, ctl) y reglas auxiliares.
├── README.md              # Descripción general del proyecto, propósito y forma de uso.
├── sensor
│   ├── sensor.c           # Implementación de la biblioteca de sensor simulado (lecturas aleatorias).
│   └── sensor.h           # Declaraciones públicas de la interfaz del sensor.
├── sensor_test            # Binario compilado de las pruebas de la biblioteca de sensores.
└── system_test            # Binario compilado de las pruebas unificadas (sensor + actuadores).

```

## Build
---

#  Caso 1: PC (x86 con soporte de multilib) → construir en 32 y 64 bits

En equipos x86 es posible compilar el mismo código con `-m32` (32 bits) y `-m64` (64 bits).

Asegurate de cambiar el archivo Makefile por el que se menciona mas adelante.

### Instrucciones de construcción

1. Asegúrate de tener las librerías multilib:

```bash
sudo apt update
sudo apt install gcc-multilib g++-multilib
```

2. Compila en **32 bits** y **64 bits**:

```bash
make clean && make 
```

Con esto obtienes dos ejecutables:

* `ctl32` (binario 32 bits).
* `ctl64` (binario 64 bits).


Para el este caso podemos utilizar el siguiente archivo Makefile

```Makefile
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
**Debido a que estamos trabajando una Raspberry pi utilizamos el caso 2 que se describe a continuación**

---

# Caso 2: Raspberry Pi aarch64 (Caso actual)

En Raspberry Pi con `uname -m = aarch64`, **no aplica `-m32/-m64`**, porque la arquitectura es ARM de 64 bits y no soporta directamente esas banderas (a menos que se instale un toolchain cruzado especial).

### Instrucciones de construcción

Solo necesitas compilar **una vez** en modo nativo:

```bash
make clean && make 
```

El binario resultante es `ctl` y estará en formato **aarch64 ELF 64-bit**, que corre nativamente en tu Raspberry.


## Ejemplo de ejecución


## `sensor_test`

**Propósito**: probar únicamente la biblioteca de sensores.
Este binario se genera a partir de `main.c` + `sensor/sensor.c`.

### Ejecución:

```bash
./sensor_test
```

### Ejemplo de salida:

```
Lecturas de prueba del sensor (10 muestras):
  lectura  1:  12.3456
  lectura  2:  -3.2100
  lectura  3:  25.8765
  ...
```

Aquí solo se verifica que el sensor simulado devuelva valores aleatorios en el rango definido (−10 a 50).

---

## `system_test`

**Propósito**: prueba **unificada de sensor + actuadores** con lógica básica.
Este binario se genera a partir de `main.c` + sensores + actuadores.

### Ejecución:

```bash
./system_test
```

### Ejemplo de salida:

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

Aquí se prueba que los actuadores reaccionan a la lectura del sensor, pero **sin retardos programados**.

---

## `ctl`

**Propósito**: controlador principal, con muestreo cada **100 ms**, uso de **monotonic time** y **apagados diferidos** (1 s buzzer, 5 s LED).
Se genera a partir de `controller/ctl.c` + sensores + actuadores.

### Ejecución:

```bash
./ctl
```

### Ejemplo de salida:

```
=== Controlador iniciado (muestreo 100ms) ===
[0.100s] Sensor=25.31 | LED=ON | Buzzer=OFF
[0.200s] Sensor=28.99 | LED=ON | Buzzer=OFF
[0.300s] Sensor=35.22 | LED=ON | Buzzer=ON
[0.400s] Sensor=32.11 | LED=ON | Buzzer=ON
[1.500s] Sensor=-2.10 | LED=ON | Buzzer=OFF   <-- buzzer apagado tras 1s
[5.600s] Sensor=-1.50 | LED=OFF | Buzzer=OFF  <-- LED apagado tras 5s
...
```

```
=== Controlador iniciado (muestreo 100ms) ===
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2863.443s] Sensor=40.41 | LED=ON | Buzzer=ON
[2863.544s] Sensor=13.66 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2863.644s] Sensor=36.99 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2863.744s] Sensor=37.91 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2863.844s] Sensor=44.70 | LED=ON | Buzzer=ON
[2863.945s] Sensor=1.85 | LED=ON | Buzzer=ON
[2864.045s] Sensor=10.11 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2864.145s] Sensor=36.09 | LED=ON | Buzzer=ON
[2864.245s] Sensor=6.67 | LED=ON | Buzzer=ON
[2864.346s] Sensor=23.24 | LED=ON | Buzzer=ON
[2864.446s] Sensor=18.64 | LED=ON | Buzzer=ON
[2864.546s] Sensor=27.73 | LED=ON | Buzzer=ON
[2864.646s] Sensor=11.89 | LED=ON | Buzzer=ON
[2864.746s] Sensor=20.80 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2864.847s] Sensor=47.13 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2864.947s] Sensor=44.97 | LED=ON | Buzzer=ON
[2865.047s] Sensor=28.14 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2865.147s] Sensor=33.04 | LED=ON | Buzzer=ON
[2865.248s] Sensor=-1.50 | LED=ON | Buzzer=ON
[2865.348s] Sensor=26.42 | LED=ON | Buzzer=ON
[2865.448s] Sensor=-9.02 | LED=ON | Buzzer=ON
[2865.548s] Sensor=4.57 | LED=ON | Buzzer=ON
[2865.649s] Sensor=-1.77 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2865.749s] Sensor=38.25 | LED=ON | Buzzer=ON
[2865.849s] Sensor=-0.60 | LED=ON | Buzzer=ON
[2865.949s] Sensor=14.06 | LED=ON | Buzzer=ON
[2866.049s] Sensor=-2.21 | LED=ON | Buzzer=ON
[2866.149s] Sensor=-3.47 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2866.250s] Sensor=49.94 | LED=ON | Buzzer=ON
[2866.350s] Sensor=3.10 | LED=ON | Buzzer=ON
[2866.450s] Sensor=20.78 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2866.550s] Sensor=40.35 | LED=ON | Buzzer=ON
[2866.651s] Sensor=26.76 | LED=ON | Buzzer=ON
[2866.751s] Sensor=7.76 | LED=ON | Buzzer=ON
[2866.851s] Sensor=28.25 | LED=ON | Buzzer=ON
[2866.951s] Sensor=21.46 | LED=ON | Buzzer=ON
[2867.051s] Sensor=19.61 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2867.152s] Sensor=48.37 | LED=ON | Buzzer=ON
[2867.252s] Sensor=7.55 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2867.352s] Sensor=36.28 | LED=ON | Buzzer=ON
[2867.453s] Sensor=21.60 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2867.553s] Sensor=36.19 | LED=ON | Buzzer=ON
[2867.653s] Sensor=14.01 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2867.753s] Sensor=43.49 | LED=ON | Buzzer=ON
[2867.853s] Sensor=7.00 | LED=ON | Buzzer=ON
[2867.954s] Sensor=11.15 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2868.054s] Sensor=38.46 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2868.154s] Sensor=45.14 | LED=ON | Buzzer=ON
[2868.255s] Sensor=-5.81 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2868.355s] Sensor=46.96 | LED=ON | Buzzer=ON
[2868.455s] Sensor=21.56 | LED=ON | Buzzer=ON
[2868.555s] Sensor=-4.84 | LED=ON | Buzzer=ON
[2868.655s] Sensor=1.53 | LED=ON | Buzzer=ON
[2868.756s] Sensor=29.79 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2868.856s] Sensor=43.41 | LED=ON | Buzzer=ON
[2868.956s] Sensor=10.93 | LED=ON | Buzzer=ON
[2869.056s] Sensor=-6.15 | LED=ON | Buzzer=ON
[2869.157s] Sensor=-8.80 | LED=ON | Buzzer=ON
[2869.257s] Sensor=17.46 | LED=ON | Buzzer=ON
[2869.357s] Sensor=-6.21 | LED=ON | Buzzer=ON
[2869.457s] Sensor=4.30 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2869.557s] Sensor=48.24 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2869.658s] Sensor=44.13 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2869.758s] Sensor=41.06 | LED=ON | Buzzer=ON
[2869.859s] Sensor=6.00 | LED=ON | Buzzer=ON
[2869.959s] Sensor=22.39 | LED=ON | Buzzer=ON
[2870.059s] Sensor=12.51 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2870.159s] Sensor=35.61 | LED=ON | Buzzer=ON
[2870.260s] Sensor=20.75 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2870.360s] Sensor=30.06 | LED=ON | Buzzer=ON
[2870.460s] Sensor=21.90 | LED=ON | Buzzer=ON
[2870.560s] Sensor=-7.64 | LED=ON | Buzzer=ON
[2870.661s] Sensor=16.26 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2870.761s] Sensor=45.91 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2870.861s] Sensor=45.85 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2870.961s] Sensor=33.26 | LED=ON | Buzzer=ON
[2871.062s] Sensor=7.06 | LED=ON | Buzzer=ON
[LED] Activado en pin 13
[BUZZER] Activado con frecuencia 2000 Hz
[2871.162s] Sensor=34.31 | LED=ON | Buzzer=ON
[2871.262s] Sensor=28.40 | LED=ON | Buzzer=ON
```

Este es el **ejecutable más completo**, que implementa el lazo de control con lógica anti-parpadeo mediante retardos.

## Inspeccion binaria

Vamos a ver ejemplos concretos de **inspección binaria con `file` y `readelf`** en dos escenarios:

1. **Raspberry Pi (aarch64)**.
2. **PC con arquitectura x86 (32/64 bits)**.

---

# Ejemplo en Raspberry Pi (aarch64)

### 1. Comprobar tipo de binario con `file`

```bash
file ctl
```

Salida en Raspberry Pi 64 bits:

```
ctl: ELF 64-bit LSB pie executable, ARM aarch64, version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-aarch64.so.1, BuildID[sha1]=861278a63f18b99716edd2986eb1913bb593dc3f, for GNU/Linux 3.7.0, not stripped
```

Aquí ves:

* ELF 64-bit LSB → binario ELF de 64 bits, Least Significant Byte primero (little endian).

* pie executable → es un PIE (Position Independent Executable), lo que significa que puede cargarse en cualquier dirección de memoria → mejora seguridad (permite ASLR).

* ARM aarch64 → arquitectura ARM de 64 bits.

* dynamically linked → usa librerías compartidas (no es estático).

* interpreter /lib/ld-linux-aarch64.so.1 → es el loader dinámico que cargará las librerías al ejecutar.

* BuildID[sha1]=... → hash único para identificar ese binario.

* for GNU/Linux 3.7.0 → binario compatible con kernels Linux ≥ 3.7.

* not stripped → conserva símbolos (nombres de funciones, variables) → útil para depuración.

---

### 2. Revisar cabecera ELF con `readelf`

```bash
readelf -h ctl
```

Salida

```
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              DYN (Position-Independent Executable file)
  Machine:                           AArch64
  Version:                           0x1
  Entry point address:               0xa80
  Start of program headers:          64 (bytes into file)
  Start of section headers:          69800 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         9
  Size of section headers:           64 (bytes)
  Number of section headers:         29
  Section header string table index: 28
```
Aquí ves:

* Class: ELF64 → confirmación de binario de 64 bits.

* Data: little endian → orden de bytes usado por ARM aarch64.

* Type: DYN → indica que es un ejecutable PIE (Dynamic), no un ejecutable estático fijo.

* Machine: AArch64 → arquitectura ARMv8 (64 bits).

* Entry point address: 0xa80 → dirección de inicio de ejecución dentro del binario.

* Program headers: 9 → número de segmentos cargables.

* Section headers: 29 → número de secciones en la tabla ELF (.text, .data, .bss, etc.).

* Section header string table index: 28 → índice que apunta a la tabla de nombres de secciones.

---

# Ejemplo en PC x86 con compilación 32 y 64 bits

Supongamos que compilamos así en una máquina x86_64:

```bash
gcc -m32 -o ctl32 controller/ctl.c ...
gcc -m64 -o ctl64 controller/ctl.c ...
```

---

### 1. Revisar con `file`

```bash
file ctl32
```

```
ctl32: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), dynamically linked,
interpreter /lib/ld-linux.so.2, for GNU/Linux 3.2.0, not stripped
```

Es un binario de **32 bits** para **Intel 80386** (x86).

```bash
file ctl64
```

```
ctl64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked,
interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, not stripped
```

Es un binario de **64 bits** para **x86-64**.

---

### 2. Cabecera ELF con `readelf`

```bash
readelf -h ctl32 | grep Class
```

```
Class:                             ELF32
Machine:                           Intel 80386
```

```bash
readelf -h ctl64 | grep Class
```

```
Class:                             ELF64
Machine:                           Advanced Micro Devices X86-64
```

## Reflexión sobre errores

### Errores de compilación

Ocurren en la **fase de compilación** → cuando `gcc` convierte el código fuente (`.c`) en un archivo objeto (`.o`).

* **Causas típicas**:

  * Errores de sintaxis (`;` faltante, `}` mal colocado).
  * Variables o funciones mal declaradas.
  * Tipos incompatibles (ejemplo: asignar un `char*` a un `int`).
  * Incluir headers inexistentes o mal escritos.

Ejemplo:

```c
int main() {
    printf("Hola mundo")   // Falta punto y coma
}
```

Error:

```
error: expected ‘;’ before ‘}’ token
```

Reflexión: **Los errores de compilación son más fáciles de detectar y corregir** porque el compilador te dice en qué línea del código fuente ocurrió el problema.

---

### Errores de enlace (linking errors)

Ocurren en la **fase de enlace** → cuando el linker (`ld`) combina varios `.o` y bibliotecas para generar el ejecutable final.

* **Causas típicas**:

  * Usar una función declarada en el header pero **no definida** en ningún `.c`.
  * No incluir la biblioteca al enlazar (ejemplo: olvidar `-lm` para funciones matemáticas como `sin`, `cos`).
  * Definir la misma función o variable global en varios `.c` (duplicados).

Ejemplo:

```c
// main.c
int main() {
    hello();
    return 0;
}
```

```c
// falta hello.c con la definición de hello()
```

Error al compilar:

```
undefined reference to `hello'
```

Reflexión: **Los errores de enlace son más engañosos** porque el código fuente parece correcto, pero el ejecutable no se puede construir.

---

### Envoltorios (Wrappers)

En programación en C, un **wrapper (envoltorio)** es una **función o módulo intermedio** que envuelve otra función/biblioteca para:

* Simplificar su uso.
* Adaptar interfaces distintas.
* Aislar dependencias de hardware o librerías externas.

Ejemplo:
En tu proyecto, podrías tener un **wrapper** de sensor:

```c
double read_temperature(void) {
    return sensor_read();   // Wrapper alrededor de la biblioteca sensor
}
```

Eso permite cambiar fácilmente la implementación interna sin afectar al resto del código.

En sistemas embebidos, los **wrappers** son útiles para:

* Aislar hardware (ejemplo: ADC real vs. sensor simulado).
* Cambiar bibliotecas de un entorno a otro (ejemplo: Linux ↔ FreeRTOS).
* Evitar errores de enlace por cambios en APIs externas → el wrapper actúa como capa de compatibilidad.



## Autor

**Brayan Avendaño Mesa**
- [@bavendanom](https://www.github.com/bavendanom)
- Curso: Programación de Sistemas Linux Embebidos - Universidad Nacional de Colombia
- 2025-2

## El programa sera un muestreador de un sensor (simulado), asi que por el momento aceptará 3 parametros:
* --interval : Cada cuánto tiempo (en segundos) leer el sensor-->(5) 
* --logfile : Ruta del archivo donde se guardarán los datos-->(/tmp/assignment_sensor.log)
* --device : Fuente del valor del sensor (archivo o fuente de datos)-->(/dev/urandom)

Ejemplo de ejecucion:

```bash
./assignment-sensor --interval 5 --logfile /tmp/sensor.log --device /dev/urandom
```

## Un ciclo:
1. Leer un número (valor) del “sensor simulado” (por ahora /dev/urandom).
2. Obtener la hora actual del sistema (en formato ISO 8601, con milisegundos).
3. Escribir una línea al logfile con el formato:

```makefile
 2025-10-06T21:45:12.123Z | 0xDEADBEEF
```
  
   
4. Esperar el tiempo de intervalo y repetir.   

## Comportamiento ante errores
| Situación                       | Qué debe hacer                                           |
| ------------------------------- | -------------------------------------------------------- |
| No puede abrir el logfile       | Intentar **fallback** → `/var/tmp/assignment_sensor.log` |
| No puede acceder al dispositivo | Mostrar error por `stderr` y **salir con código 2**      |
| Recibe `SIGTERM`                | Cerrar archivo, hacer flush y salir con código 0         |
| Error de escritura              | Mostrar error, reintentar si es posible, luego abortar   |

## Qué se considera “correcto” en ejecución

* Cada línea en el logfile debe estar completa y terminada con salto de línea (\n).

* No debe haber líneas partidas o mezcladas.

* El proceso debe cerrarse limpio al hacer `systemctl stop`.
## Autor

**Brayan Avendaño Mesa**
- [@bavendanom](https://www.github.com/bavendanom)
- Curso: Programación de Sistemas Linux Embebidos - Universidad Nacional de Colombia
- 2025-2

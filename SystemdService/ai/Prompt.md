
# Prompt

## `Systemd Service`:


### 1. Estructura del proyecto:

```makefile
assignment-sensor/
├─ src/ código fuente (C)
│  └─ main.c
├─ systemd/  unit file del servicio
│  └─ assignment-sensor.service
├─ tests/ scripts de prueba
│  └─ smoke-test.sh
├─ ai/ logs y reflexiones del uso de IA
│  ├─ prompt-log.md
│  ├─ reflection.md
│  └─ provenance.json
├─ Makefile
└─ README.md
├─ build/ carpeta de compilados
```

### 2. Descripción general del funcionamiento

El programa:

1. Lee periódicamente un valor numérico del dispositivo o fuente definida (por defecto `/dev/urandom`).
2. Obtiene la hora actual del sistema en formato **ISO 8601** con precisión de milisegundos.
3. Escribe en el archivo de log una línea con el formato:
    
    ```
    YYYY-MM-DDTHH:MM:SS.mmmZ | 0xXXXXXXXX
    ```
    
    donde:
    
    - `YYYY-MM-DDTHH:MM:SS.mmmZ` → fecha y hora UTC.
    - `0xXXXXXXXX` → valor hexadecimal leído del dispositivo debe ser de 4 bytes (como `0xDEADBEEF`).
4. Espera el tiempo indicado por el parámetro `-interval` y repite el ciclo.
5. Al recibir la señal `SIGTERM` (por ejemplo, al ejecutar `systemctl stop`), el programa finaliza limpiamente cerrando el archivo de log y garantizando que todos los datos pendientes se escribieron correctamente.

### 3. Parámetros de ejecución

El programa acepta los siguientes argumentos por línea de comandos:

| Parámetro | Descripción | Valor por defecto | Obligatorio |
| --- | --- | --- | --- |
| `--interval <segundos>` | Tiempo de muestreo entre lecturas sucesivas del sensor. | `5` | No |
| `--logfile <ruta>` | Ruta completa al archivo donde se almacenarán los datos. | `/tmp/assignment_sensor.log` | No |
| `--device <ruta>` | Fuente de datos o dispositivo a leer. | `/dev/urandom` | No |

Ejemplo de uso:

```bash
./assignment-sensor --interval 2 --logfile /tmp/sensor.log --device /dev/urandom
```

### 4. Formato del archivo de log

Cada línea del archivo contendrá un registro individual con el siguiente formato:

```
2025-10-06T22:11:25.123Z | 0xDEADBEEF
2025-10-06T22:11:27.125Z | 0xCAFEBABE
```

**Características del log:**

- Cada línea se escribe **en una sola operación atómica** (`write()`), evitando líneas parciales.
- El archivo se abre con la bandera `O_APPEND`, garantizando que los datos se agreguen siempre al final.
- Si el archivo no puede crearse o escribirse en `/tmp`, se usará un **path de respaldo (fallback)** en `/var/tmp/assignment_sensor.log`.
- El programa realiza **flush y cierre seguro** al finalizar.

---

### 5. Comportamiento ante errores

| Situación | Acción del programa | Código de salida | Qué debe hacer |
| --- | --- | --- | --- |
| No se puede abrir el archivo de log y el fallback también falla | Muestra error en `stderr` y termina | `2` | Intentar **fallback** → `/var/tmp/assignment_sensor.log` |
| No se puede acceder al dispositivo o fuente de datos | Muestra error y termina | `2` | Mostrar error por `stderr` y **salir con código 2** |
| Error de escritura en log (temporal) | Muestra advertencia, intenta una vez más | `1` | Mostrar error, reintentar si es posible, luego abortar  |
| Recepción de señal `SIGTERM` | Termina de forma controlada, cierra archivos y sincroniza datos | `0` | Cerrar archivo, hacer flush y salir con código 0  |

---

### 6. Comportamiento ante señales

El programa implementa un manejador de señales para `SIGTERM`:

- Al recibirla, no interrumpe bruscamente el proceso.
- Establece una bandera interna (`stop = 1`) que se evalúa en el siguiente ciclo del bucle principal.
- Realiza:
    - `fsync()` para asegurar que los datos pendientes lleguen al disco.
    - `close()` para liberar correctamente el descriptor de archivo.

Esto asegura una **terminación limpia** sin corrupción del log ni pérdida de información.

### 7. Ejemplo del ciclo de ejecución

1. Inicio del programa → apertura del logfile y del dispositivo.
2. Bucle principal:
    - Obtener timestamp.
    - Leer valor del sensor.
    - Formatear línea completa.
    - Escribir en log (operación atómica).
    - Dormir `interval` segundos.
3. Al recibir `SIGTERM` → salir del bucle, sincronizar y cerrar archivo.

---

### 8. Comportamiento como servicio `systemd`

Cuando se ejecute como servicio:

- El archivo unit (`assignment-sensor.service`) incluirá:
    
    ```
    [Unit]
    Description=Mock sensor logger
    After=network.target
    
    [Service]
    Type=simple
    ExecStart=/usr/local/bin/assignment-sensor --interval 5 --logfile /tmp/assignment_sensor.log
    Restart=on-failure
    RestartSec=2
    
    [Install]
    WantedBy=multi-user.target
    ```
    
- El servicio se iniciará al alcanzar el `multi-user.target` (modo multiusuario).
- Deberá detenerse correctamente mediante:
    
    ```bash
    sudo systemctl stop assignment-sensor.service
    ```
    
- Los registros podrán visualizarse mediante:
    
    ```bash
    journalctl -u assignment-sensor.service -f
    ```
    

---

### 9. Criterios de finalización correcta

El programa se considera **correctamente implementado y funcional** cuando:

1. Es capaz de crear su log en `/tmp` o fallback en `/var/tmp`.
2. Registra datos de forma continua y sin líneas corruptas.
3. Termina limpiamente al recibir `SIGTERM`.
4. Puede ejecutarse tanto manualmente como mediante `systemd`.
5. Se integra sin errores al arranque del sistema (`multi-user.target`).
6. Supera las pruebas definidas en `tests/`.

### 10. Archivos generados

| Archivo | Descripción |
| --- | --- |
| `/tmp/assignment_sensor.log` | Archivo principal de registros. |
| `/var/tmp/assignment_sensor.log` | Log alternativo (fallback). |
| `/usr/local/bin/assignment-sensor` | Binario compilado del programa. |
| `/etc/systemd/system/assignment-sensor.service` | Unit file del servicio. |

---

### 11. Notas adicionales

- El programa está diseñado para ejecutarse en sistemas Linux con `systemd`, especialmente **Raspberry Pi 3 B+**.
- Los valores obtenidos no representan datos físicos reales: el uso de `/dev/urandom` cumple la función de un *mock sensor*.
- Todo el código deberá compilarse con `gcc` o una herramienta equivalente disponible en la Pi.



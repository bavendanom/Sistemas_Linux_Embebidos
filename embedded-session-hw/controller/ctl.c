#define _POSIX_C_SOURCE 200809L
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
        struct timespec req = {0};
        req.tv_sec = 0;
        req.tv_nsec = 100000000L; // 100 ms
        nanosleep(&req, NULL);
    }

    /* Liberar recursos (aunque aquí nunca se alcanzará) */
    destroy_led_actuator(led);
    destroy_buzzer_actuator(buzzer);

    return 0;
}

#include <stdio.h>
#include <time.h>
#include <unistd.h>  // Para usleep y nanosleep
#include "sensor.h"
#include "actuator.h"

// Declaraciones externas
extern actuator_t create_led_actuator(void);
extern actuator_t create_buzzer_actuator(void);

// Función auxiliar para timestamp
void print_log(double value, int led_state, int buzzer_state) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buffer[9];
    strftime(buffer, 9, "%H:%M:%S", tm_info);
    printf("[%s] Valor: %.2f | LED: %s | Buzzer: %s\n",
           buffer,
           value,
           led_state ? "ON" : "OFF",
           buzzer_state ? "ON" : "OFF");
}

int main(void) {
    sensor_init();
    actuator_t led = create_led_actuator();
    actuator_t buzzer = create_buzzer_actuator();

    double threshold = 50.0;
    struct timespec now, buzzer_off_time = {0}, led_off_time = {0};
    int buzzer_timer_active = 0, led_timer_active = 0;

    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        double value = sensor_read();

        // Comprobación si el valor supera el umbral
        if (value >= threshold) {
            buzzer.activate(buzzer.params);
            led.activate(led.params);
            buzzer_timer_active = 0;
            led_timer_active = 0;
        } else {
            // Iniciar temporizadores para el buzzer y LED
            if (!buzzer_timer_active) {
                buzzer_off_time = now;
                buzzer_off_time.tv_sec += 1;  // Se apaga después de 1 segundo
                buzzer_timer_active = 1;
            }
            if (!led_timer_active) {
                led_off_time = now;
                led_off_time.tv_sec += 5;  // Se apaga después de 5 segundos
                led_timer_active = 1;
            }
        }

        // Desactivar buzzer cuando el tiempo haya pasado
        if (buzzer_timer_active) {
            if (now.tv_sec > buzzer_off_time.tv_sec ||
                (now.tv_sec == buzzer_off_time.tv_sec && now.tv_nsec >= buzzer_off_time.tv_nsec)) {
                printf("[Buzzer] Apagado a las %ld:%ld\n", now.tv_sec, now.tv_nsec);  // Depuración
                buzzer.deactivate(buzzer.params);
                buzzer_timer_active = 0;
            }
        }

        // Desactivar LED cuando el tiempo haya pasado
        if (led_timer_active) {
            if (now.tv_sec > led_off_time.tv_sec ||
                (now.tv_sec == led_off_time.tv_sec && now.tv_nsec >= led_off_time.tv_nsec)) {
                printf("[LED] Apagado a las %ld:%ld\n", now.tv_sec, now.tv_nsec);  // Depuración
                led.deactivate(led.params);
                led_timer_active = 0;
            }
        }

        // Imprimir el log con los valores y el estado de los dispositivos
        print_log(value, led.status(led.params), buzzer.status(buzzer.params));

        // Espera de 100ms (usando nanosleep)
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000L;  // 100 ms
        nanosleep(&ts, NULL);
    }

    return 0;
}

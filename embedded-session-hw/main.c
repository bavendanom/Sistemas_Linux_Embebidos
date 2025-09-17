#include <stdio.h>
#include "sensor/sensor.h"

int main(void) {
    sensor_init();

    for (int i = 0; i < 5; i++) {
        double value = sensor_read();
        printf("Lectura %d: %.2f\n", i + 1, value);
    }

    return 0;
}

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

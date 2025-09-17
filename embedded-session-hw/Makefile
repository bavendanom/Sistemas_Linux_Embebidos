CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LIBRARY_PATH = ./actuators

# Archivos fuente
SOURCES = main.c $(LIBRARY_PATH)/led_actuator.c $(LIBRARY_PATH)/buzzer_actuator.c sensor/sensor.c

# Archivos objeto
OBJECTS = $(SOURCES:.c=.o)

# Nombre del ejecutable
EXEC = sensor_test

# Regla para compilar el ejecutable
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

# Reglas para compilar los archivos fuente
%.o: %.c
	$(CC) $(CFLAGS) -I$(LIBRARY_PATH) -I./sensor -c $< -o $@

# Regla para limpiar los archivos objeto y ejecutable
clean:
	rm -f $(OBJECTS) $(EXEC)

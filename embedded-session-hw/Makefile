CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: sensor_test

sensor_test: sensor/sensor.o main.o
	$(CC) $(CFLAGS) sensor/sensor.o main.o -o sensor_test

sensor.o: sensor/sensor.c sensor/sensor.h
	$(CC) $(CFLAGS) -c sensor.c

main.o: main.c sensor/sensor.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o sensor_test

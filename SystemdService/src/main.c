#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>

#define DEFAULT_INTERVAL 5
#define DEFAULT_LOGFILE "/tmp/assignment_sensor.log"
#define FALLBACK_LOGFILE "/var/tmp/assignment_sensor.log"
#define DEFAULT_DEVICE "/dev/urandom"
#define BUFFER_SIZE 256

// Variables globales para manejo de señales
volatile sig_atomic_t stop_requested = 0;
int log_fd = -1;
int device_fd = -1;

// Manejo de señales
void signal_handler(int sig) {
    if (sig == SIGTERM) {
        stop_requested = 1;
    }
}


// Obtener timestamp en formato ISO 8601 con milisegundos
void get_iso8601_timestamp(char *buffer, size_t buffer_size) {
    struct timespec ts;
    struct tm tm_info;
    
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("clock_gettime failed");
        snprintf(buffer, buffer_size, "1970-01-01T00:00:00.000Z");
        return;
    }
    
    if (gmtime_r(&ts.tv_sec, &tm_info) == NULL) {
        perror("gmtime_r failed");
        snprintf(buffer, buffer_size, "1970-01-01T00:00:00.000Z");
        return;
    }
    
    // Calcular milisegundos de forma segura
    int milliseconds = (int)(ts.tv_nsec / 1000000);
    
    // Formatear todo en una sola operación
    snprintf(buffer, buffer_size, 
             "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday,
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec,
             milliseconds);
}

// Intentar abrir archivo de log con fallback
int open_logfile(const char *primary_path, const char *fallback_path) {
    int fd = open(primary_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        fprintf(stderr, "Warning: Cannot open primary logfile '%s': %s\n", 
                primary_path, strerror(errno));
        
        fd = open(fallback_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1) {
            fprintf(stderr, "Error: Cannot open fallback logfile '%s': %s\n", 
                    fallback_path, strerror(errno));
            return -1;
        }
        
        fprintf(stderr, "Info: Using fallback logfile '%s'\n", fallback_path);
    }
    
    return fd;
}

// Leer valor del dispositivo (4 bytes) y convertirlo a hexadecimal
int read_sensor_value(int fd, char *hex_buffer, size_t buffer_size) {
    unsigned int value;
    ssize_t bytes_read = read(fd, &value, sizeof(value));
    
    if (bytes_read != sizeof(value)) {
        return -1;
    }
    
    snprintf(hex_buffer, buffer_size, "0x%08X", value);
    return 0;
}

// Escribir línea en el log de forma atómica
int write_log_entry(int fd, const char *timestamp, const char *sensor_value) {
    char log_line[BUFFER_SIZE];
    int len = snprintf(log_line, sizeof(log_line), "%s | %s\n", 
                       timestamp, sensor_value);
    
    if (len <= 0 || len >= (int)sizeof(log_line)) {
        return -1;
    }
    
    ssize_t written = write(fd, log_line, len);
    if (written != len) {
        return -1;
    }
    
    return 0;
}

// Limpieza y cierre seguro
void cleanup_resources() {
    if (log_fd != -1) {
        fsync(log_fd);  // Asegurar que los datos lleguen al disco
        close(log_fd);
        log_fd = -1;
    }
    
    if (device_fd != -1) {
        close(device_fd);
        device_fd = -1;
    }
}

// Mostrar ayuda
void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  --interval <seconds>  Sampling interval (default: %d)\n", DEFAULT_INTERVAL);
    printf("  --logfile <path>      Log file path (default: %s)\n", DEFAULT_LOGFILE);
    printf("  --device <path>       Sensor device (default: %s)\n", DEFAULT_DEVICE);
    printf("  --help                Show this help message\n");
}

// Parsear argumentos de línea de comandos
void parse_arguments(int argc, char *argv[], int *interval, char **logfile, char **device) {
    *interval = DEFAULT_INTERVAL;
    *logfile = DEFAULT_LOGFILE;
    *device = DEFAULT_DEVICE;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--interval") == 0 && i + 1 < argc) {
            *interval = atoi(argv[++i]);
            if (*interval <= 0) {
                fprintf(stderr, "Error: Interval must be positive\n");
                exit(2);
            }
        } else if (strcmp(argv[i], "--logfile") == 0 && i + 1 < argc) {
            *logfile = argv[++i];
        } else if (strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
            *device = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            exit(2);
        }
    }
}

int main(int argc, char *argv[]) {
    int interval;
    char *logfile_path, *device_path;
    
    // Parsear argumentos
    parse_arguments(argc, argv, &interval, &logfile_path, &device_path);
    
    // Configurar manejador de señales
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        fprintf(stderr, "Error: Cannot set signal handler: %s\n", strerror(errno));
        exit(2);
    }
    
    // Abrir archivo de log
    log_fd = open_logfile(logfile_path, FALLBACK_LOGFILE);
    if (log_fd == -1) {
        fprintf(stderr, "Error: Cannot open any logfile\n");
        exit(2);
    }
    
    // Abrir dispositivo
    device_fd = open(device_path, O_RDONLY);
    if (device_fd == -1) {
        fprintf(stderr, "Error: Cannot open device '%s': %s\n", 
                device_path, strerror(errno));
        cleanup_resources();
        exit(2);
    }
    
    printf("Sensor logger started:\n");
    printf("  Interval: %d seconds\n", interval);
    printf("  Logfile: %s\n", (log_fd != -1) ? logfile_path : FALLBACK_LOGFILE);
    printf("  Device: %s\n", device_path);
    printf("Press Ctrl+C or send SIGTERM to stop\n");
    
    // Bucle principal
    char timestamp[64];
    char sensor_value[16];
    
    while (!stop_requested) {
        // Obtener timestamp
        get_iso8601_timestamp(timestamp, sizeof(timestamp));
        
        // Leer valor del sensor
        if (read_sensor_value(device_fd, sensor_value, sizeof(sensor_value)) == -1) {
            fprintf(stderr, "Error: Cannot read from device '%s': %s\n", 
                    device_path, strerror(errno));
            cleanup_resources();
            exit(2);
        }
        
        // Escribir en el log
        if (write_log_entry(log_fd, timestamp, sensor_value) == -1) {
            fprintf(stderr, "Warning: Write error, retrying...\n");
            
            // Reintentar una vez
            if (write_log_entry(log_fd, timestamp, sensor_value) == -1) {
                fprintf(stderr, "Error: Persistent write error, aborting\n");
                cleanup_resources();
                exit(1);
            }
        }
        
        // Esperar el intervalo especificado
        for (int i = 0; i < interval && !stop_requested; i++) {
            sleep(1);
        }
    }
    
    // Terminación limpia
    printf("Received SIGTERM, shutting down cleanly...\n");
    cleanup_resources();
    printf("Sensor logger stopped successfully\n");
    
    return 0;
}
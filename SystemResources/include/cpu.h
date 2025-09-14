#ifndef CPU_H
#define CPU_H

#include <stddef.h>

#define CPU_MAX_CORES 64  // suficiente para RPi y de sobra

typedef struct {
    char model[128];      // "ARMv7 Processor rev 4 (v7l)" o similar
    char hardware[64];    // "BCM2835" / "BCM2837" (SoC)
    char arch[32];        // uname -m (p. ej. "armv7l", "aarch64")
    int  cores_config;    // núcleos configurados (posibles)
    int  cores_online;    // núcleos activos
} CPUInfo;

typedef struct {
    int    cores;                     // núcleos medidos
    double per_core[CPU_MAX_CORES];   // uso [0..1] por core
    double avg;                       // promedio [0..1]
    int    ready;                     // 0 = primera muestra (aún sin delta), 1 = listo
} CPUUsage;

// --- Actividad 3 y 4 ---
int cpu_read_info(CPUInfo *out);

// --- Actividad 5 (usa deltas de /proc/stat internamente, listo para llamar cada 2 s) ---
int cpu_read_usage(CPUUsage *out);

#endif // CPU_H

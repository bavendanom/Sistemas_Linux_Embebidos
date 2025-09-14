#include "memory.h"
#include <stdio.h>
#include <string.h>

int mem_total_kib(long *out_kib) {
    if (!out_kib) return 1;
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return 2;

    char line[256];
    long value_kb = -1;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            long kb = -1;
            if (sscanf(line, "MemTotal: %ld kB", &kb) == 1) {
                value_kb = kb;
                break;
            }
        }
    }
    fclose(f);
    if (value_kb < 0) return 3;
    *out_kib = value_kb;
    return 0;
}

void format_bytes_from_kib(long kib, char *buf, size_t buflen) {
    if (!buf || buflen == 0) return;
    double kib_d = (double)kib;     // cast explícito para -Wconversion
    double mib = kib_d / 1024.0;
    double gib = mib / 1024.0;

    if (gib >= 1.0) {
        snprintf(buf, buflen, "%.2f GiB", gib);
    } else if (mib >= 1.0) {
        snprintf(buf, buflen, "%.2f MiB", mib);
    } else {
        snprintf(buf, buflen, "%ld KiB", kib);
    }
}

// --- NUEVO: Actividad 2 ---
int mem_read_stats(MemStats *out) {
    if (!out) return 1;

    // Inicializa en "no disponible"
    long mem_total = -1, mem_avail = -1;
    long mem_free = -1, buffers = -1, cached = -1, sreclaimable = -1, shmem = -1;
    long swap_total = -1, swap_free = -1;

    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return 2;

    char key[64];
    long value_kb;
    char unit[16];

    // Leemos claves importantes. El archivo tiene muchas líneas; iteramos todas.
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%63[^:]: %ld %15s", key, &value_kb, unit) == 3) {
            if (strcmp(key, "MemTotal") == 0)        mem_total = value_kb;
            else if (strcmp(key, "MemAvailable") == 0) mem_avail = value_kb;
            else if (strcmp(key, "MemFree") == 0)      mem_free = value_kb;
            else if (strcmp(key, "Buffers") == 0)      buffers = value_kb;
            else if (strcmp(key, "Cached") == 0)       cached = value_kb;
            else if (strcmp(key, "SReclaimable") == 0) sreclaimable = value_kb;
            else if (strcmp(key, "Shmem") == 0)        shmem = value_kb;
            else if (strcmp(key, "SwapTotal") == 0)    swap_total = value_kb;
            else if (strcmp(key, "SwapFree") == 0)     swap_free = value_kb;
        }
    }
    fclose(f);

    if (mem_total < 0) return 3; // sin MemTotal no podemos continuar

    // Calcular MemAvailable si no está (fallback clásico):
    // available ≈ MemFree + Buffers + Cached + SReclaimable - Shmem
    if (mem_avail < 0) {
        long sum = 0;
        if (mem_free > 0)       sum += mem_free;
        if (buffers > 0)        sum += buffers;
        if (cached > 0)         sum += cached;
        if (sreclaimable > 0)   sum += sreclaimable;
        if (shmem > 0)          sum -= shmem;
        if (sum < 0) sum = 0;
        mem_avail = sum;
    }

    long used = mem_total - mem_avail;
    if (used < 0) used = 0;

    long s_total = (swap_total > 0) ? swap_total : 0;
    long s_free  = (swap_free  > 0) ? swap_free  : 0;
    long s_used  = (s_total > s_free) ? (s_total - s_free) : 0;

    out->total_kib = mem_total;
    out->avail_kib = mem_avail;
    out->used_kib  = used;

    out->swap_total_kib = s_total;
    out->swap_free_kib  = s_free;
    out->swap_used_kib  = s_used;

    out->used_ratio = (mem_total > 0) ? ((double)used / (double)mem_total) : 0.0;
    out->swap_used_ratio = (s_total > 0) ? ((double)s_used / (double)s_total) : 0.0;

    return 0;
}



#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

typedef struct {
    long total_kib;        // RAM total
    long avail_kib;        // RAM disponible (estimada por kernel)
    long used_kib;         // RAM usada (total - available)

    long swap_total_kib;   // Swap total
    long swap_free_kib;    // Swap libre
    long swap_used_kib;    // Swap usada (total - free)

    double used_ratio;       // RAM usada / total   (0..1)
    double swap_used_ratio;  // Swap usada / total  (0..1)
} MemStats;

// Actividad 1 (ya existente): memoria instalada
int mem_total_kib(long *out_kib);

// Utilidad: formatear KiB -> "X.Y GiB/MiB/KiB"
void format_bytes_from_kib(long kib, char *buf, size_t buflen);

// Actividad 2: leer RAM/SWAP usadas y totales.
// Devuelve 0 si OK. No falla si no hay swap (deja swap_total_kib=0).
int mem_read_stats(MemStats *out);

#endif // MEMORY_H

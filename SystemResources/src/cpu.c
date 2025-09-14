#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>         // sysconf
#include <sys/utsname.h>    // uname

// ---------- Utils ----------
static void trim(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n && (s[n-1] == '\n' || s[n-1] == '\r' || s[n-1] == ' ' || s[n-1] == '\t')) {
        s[--n] = '\0';
    }
    size_t i = 0;
    while (s[i] == ' ' || s[i] == '\t') i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);
}
static void safe_copy(char *dst, size_t dstsz, const char *src) {
    if (!dst || dstsz == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    size_t len = strlen(src);
    if (len >= dstsz) len = dstsz - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

// ---------- Actividad 3 y 4 ----------
int cpu_read_info(CPUInfo *out) {
    if (!out) return 1;
    memset(out, 0, sizeof(*out));

    // Arquitectura (uname -m)
    struct utsname un;
    if (uname(&un) == 0) {
        safe_copy(out->arch, sizeof(out->arch), un.machine);
    } else {
        safe_copy(out->arch, sizeof(out->arch), "unknown");
    }

    // Núcleos configurados y en línea
    long conf  = sysconf(_SC_NPROCESSORS_CONF);
    long onln  = sysconf(_SC_NPROCESSORS_ONLN);
    out->cores_config = (conf > 0) ? (int)conf : 0;
    out->cores_online = (onln > 0) ? (int)onln : 0;

    // Modelo / Hardware desde /proc/cpuinfo (depende de ARM/64)
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (!f) return 0;  // seguimos con lo que tenemos

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64], value[192];
        memset(key, 0, sizeof(key));
        memset(value, 0, sizeof(value));

        // formato típico: "model name\t: ARMv7 Processor ..."
        char *colon = strchr(line, ':');
        if (!colon) continue;
        size_t klen = (size_t)(colon - line);
        if (klen >= sizeof(key)) klen = sizeof(key) - 1;
        memcpy(key, line, klen);
        key[klen] = '\0';
        trim(key);

        safe_copy(value, sizeof(value), colon + 1);
        trim(value);

        if (strcasecmp(key, "model name") == 0 || strcasecmp(key, "Model name") == 0 || strcasecmp(key, "Processor") == 0) {
	    if (out->model[0] == '\0') safe_copy(out->model, sizeof(out->model), value);
	} else if (strcasecmp(key, "Hardware") == 0) {
    	   if (out->hardware[0] == '\0') safe_copy(out->hardware, sizeof(out->hardware), value);
	}

    }
    fclose(f);

    // Rellenos si faltó algo
    if (out->model[0] == '\0') safe_copy(out->model, sizeof(out->model), "N/D");
    if (out->hardware[0] == '\0') safe_copy(out->hardware, sizeof(out->hardware), "N/D");

    return 0;
}

// ---------- Actividad 5 ----------
// guardamos snapshot previo para deltas
#define MAX_CPUS_INTERNAL 128
static unsigned long long prev_total[MAX_CPUS_INTERNAL];
static unsigned long long prev_idle [MAX_CPUS_INTERNAL];
static int prev_cores = -1;
static int initialized = 0;

int cpu_read_usage(CPUUsage *out) {
    if (!out) return 1;
    memset(out, 0, sizeof(*out));

    FILE *f = fopen("/proc/stat", "r");
    if (!f) {
        out->ready = 0;
        return 2;
    }

    // saltamos la línea "cpu " (global)
    char line[512];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return 3; }

    int core_idx = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "cpu", 3) != 0) break;
        if (line[3] < '0' || line[3] > '9') break;   // no es cpuN

        // Campos: user nice system idle iowait irq softirq steal guest guest_nice
        unsigned long long user=0, nice=0, system=0, idle=0, iowait=0, irq=0, softirq=0, steal=0, guest=0, guest_nice=0;
        int scanned = sscanf(line, "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                             &core_idx, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
        if (scanned < 5) break; // al menos hasta idle

        unsigned long long idle_all = idle + iowait;
        unsigned long long non_idle = user + nice + system + irq + softirq + steal;
        unsigned long long total = idle_all + non_idle;

        if (!initialized) {
            if (core_idx < MAX_CPUS_INTERNAL) {
                prev_total[core_idx] = total;
                prev_idle [core_idx] = idle_all;
            }
        } else {
            if (core_idx < MAX_CPUS_INTERNAL) {
                unsigned long long dt = total > prev_total[core_idx] ? (total - prev_total[core_idx]) : 0ULL;
                unsigned long long di = idle_all > prev_idle[core_idx] ? (idle_all - prev_idle[core_idx]) : 0ULL;
                double ratio = 0.0;
                if (dt > 0ULL) ratio = (double)(dt - di) / (double)dt;
                if (out->cores < CPU_MAX_CORES) {
                    out->per_core[out->cores] = (ratio < 0.0) ? 0.0 : (ratio > 1.0 ? 1.0 : ratio);
                    out->avg += out->per_core[out->cores];
                    out->cores++;
                }
                prev_total[core_idx] = total;
                prev_idle [core_idx] = idle_all;
            }
        }
    }
    fclose(f);

    if (!initialized) {
        initialized = 1;
        prev_cores = core_idx + 1;
        out->ready = 0; // primera lectura: aún no hay delta
        return 0;
    }

    if (out->cores > 0) out->avg /= (double)out->cores;
    out->ready = 1;
    return 0;
}

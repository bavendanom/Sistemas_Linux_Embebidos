#include "memory.h"
#include "cpu.h"
#include "tui.h"
#include <stdio.h>

static CPUInfo g_info;  // cache: no cambia frecuentemente

static void draw_once(void) {
    MemStats mem;
    CPUUsage usage;

    int rc1 = mem_read_stats(&mem);
    int rc2 = cpu_read_usage(&usage);  // primera llamada del programa marcará ready=0

    if (rc1 != 0) {
        tui_draw_memory("ERROR leyendo /proc/meminfo");
        return;
    }
    (void)rc2; // si falla /proc/stat, simplemente mostrará "calculando…"

    tui_draw_system_dashboard(&mem, &g_info, &usage);
}

int main(void) {
    if (tui_init() != 0) {
        fprintf(stderr, "No se pudo inicializar la TUI (ncurses).\n");
        return 1;
    }

    // CPU info una sola vez
    cpu_read_info(&g_info);

    draw_once();

    for (;;) {
        int ch = tui_getch();     // timeout 2 s ya activo
        if (ch == 'q') break;
        if (ch == 'r' || tui_was_timeout(ch)) {
            draw_once();
        }
    }

    tui_end();
    return 0;
}


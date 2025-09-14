#include "tui.h"
#include <ncursesw/ncurses.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <time.h>

// --- Estado de layout persistente ---
static WINDOW *g_win_mem = NULL;
static WINDOW *g_win_cpu = NULL;

static int g_rows = 0, g_cols = 0;
static int g_mem_h = 0, g_cpu_h = 0;
static int g_panel_w = 0, g_panel_x = 0;
static int g_mem_y = 0, g_cpu_y = 0;

static void destroy_windows(void) {
    if (g_win_mem) { delwin(g_win_mem); g_win_mem = NULL; }
    if (g_win_cpu) { delwin(g_win_cpu); g_win_cpu = NULL; }
}

// Calcula/recrea el layout si cambió el tamaño de terminal
static void ensure_layout(void) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    if (rows == g_rows && cols == g_cols && g_win_mem && g_win_cpu) return;

    // Guarda y recrea
    g_rows = rows; g_cols = cols;
    destroy_windows();

    const int header_lines = 5;
    const int footer_lines = 2;
    const int gap_between  = 1;

    int available = g_rows - header_lines - footer_lines - gap_between;
    if (available < 12 || g_cols < 40) {
        // Demasiado pequeño → no usamos ventanas (fallback)
        return;
    }

    // 40% memoria, 60% CPU con mínimos
    g_mem_h = (available * 2) / 5; if (g_mem_h < 8) g_mem_h = 8;
    g_cpu_h = available - g_mem_h; if (g_cpu_h < 8) g_cpu_h = 8;

    g_panel_w = g_cols - 4;
    g_panel_x = 2;
    g_mem_y   = header_lines;
    g_cpu_y   = g_mem_y + g_mem_h + gap_between;

    // Último ajuste si no cabe exacto
    int overflow = (g_cpu_y + g_cpu_h + footer_lines) - g_rows;
    if (overflow > 0) { g_cpu_h -= overflow; if (g_cpu_h < 6) g_cpu_h = 6; }

    // Crea ventanas (si no caben, quedarán NULL y usaremos fallback)
    if (g_panel_w >= 20 && g_mem_h >= 6)
        g_win_mem = newwin(g_mem_h, g_panel_w, g_mem_y, g_panel_x);
    if (g_panel_w >= 20 && g_cpu_h >= 6)
        g_win_cpu = newwin(g_cpu_h, g_panel_w, g_cpu_y, g_panel_x);
}


// Prototipos helpers (deben ir antes de cualquier uso)
static void draw_bar(int y, int x, int width, double ratio);
static void draw_bar_win(WINDOW *w, int y, int x, int width, double ratio);

// ==================== Inicialización / cierre ====================

int tui_init(void) {
    // Habilita UTF-8 según el locale del sistema (para tildes, “—”, etc.)
    setlocale(LC_ALL, "");

    if (initscr() == NULL) return 1;

    if (has_colors()) {
        start_color();
        // 1: títulos, 2: amarillo (medio), 3: verde (OK), 4: rojo (alto), 5: bordes/azul
        init_pair(1, COLOR_CYAN,   COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_GREEN,  COLOR_BLACK);
        init_pair(4, COLOR_RED,    COLOR_BLACK);
        init_pair(5, COLOR_BLUE,   COLOR_BLACK);
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(2000); // auto-refresh cada 2 s
    return 0;
}

void tui_end(void) {
    destroy_windows();
    endwin();
}

// ==================== Vistas antiguas (por compatibilidad) ====================

void tui_draw_memory(const char *installed_str) {
    erase();
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    const char *title = "Raspberry Pi — System Info (TUI)";
    const char *subtitle = "[Actividad 1] Memoria instalada";
    const char *hint = "Auto-refresh 2 s — 'r' refrescar ahora, 'q' salir";

    int title_x = (cols - (int)strlen(title)) / 2;
    int sub_x   = (cols - (int)strlen(subtitle)) / 2;

    if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, title_x > 0 ? title_x : 0, "%s", title);
    if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);

    if (has_colors()) attron(COLOR_PAIR(2));
    mvprintw(3, sub_x > 0 ? sub_x : 0, "%s", subtitle);
    if (has_colors()) attroff(COLOR_PAIR(2));

    char line[128];
    snprintf(line, sizeof(line), "Memoria instalada: %s", installed_str);
    int line_x = (cols - (int)strlen(line)) / 2;
    mvprintw(rows/2, line_x > 0 ? line_x : 0, "%s", line);

    mvprintw(rows - 2, 2, "%s", hint);
    refresh();
}

void tui_draw_memory_usage(const MemStats *st) {
    erase();
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    const char *title = "Raspberry Pi — System Info (TUI)";
    const char *subtitle = "[Actividad 2] Uso de memoria física y virtual";
    const char *hint = "Auto-refresh 2 s — 'r' refrescar ahora, 'q' salir";

    int title_x = (cols - (int)strlen(title)) / 2;
    int sub_x   = (cols - (int)strlen(subtitle)) / 2;

    if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, title_x > 0 ? title_x : 0, "%s", title);
    if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);

    if (has_colors()) attron(COLOR_PAIR(2));
    mvprintw(3, sub_x > 0 ? sub_x : 0, "%s", subtitle);
    if (has_colors()) attroff(COLOR_PAIR(2));

    char t_total[64], t_used[64], t_avail[64], s_total[64], s_used[64], s_free[64];
    format_bytes_from_kib(st->total_kib, t_total, sizeof(t_total));
    format_bytes_from_kib(st->used_kib,  t_used,  sizeof(t_used));
    format_bytes_from_kib(st->avail_kib, t_avail, sizeof(t_avail));
    format_bytes_from_kib(st->swap_total_kib, s_total, sizeof(s_total));
    format_bytes_from_kib(st->swap_used_kib,  s_used,  sizeof(s_used));
    format_bytes_from_kib(st->swap_free_kib,  s_free,  sizeof(s_free));

    int y = 6;
    mvprintw(y++, 4, "RAM total: %s | usada: %s | disponible: %s | uso: %.1f%%",
             t_total, t_used, t_avail, st->used_ratio * 100.0);

    int bar_width = cols - 8;
    if (bar_width < 20) bar_width = 20;
    draw_bar(y++, 4, bar_width, st->used_ratio);

    y += 1;

    mvprintw(y++, 4, "SWAP total: %s | usada: %s | libre: %s | uso: %.1f%%",
             s_total, s_used, s_free,
             (st->swap_total_kib > 0 ? st->swap_used_ratio * 100.0 : 0.0));
    double r = (st->swap_total_kib > 0) ? st->swap_used_ratio : 0.0;
    draw_bar(y++, 4, bar_width, r);

    mvprintw(rows - 2, 2, "%s", hint);
    refresh();
}

// ==================== Dashboard mejorado (ventanas y cajas) ====================

void tui_draw_system_dashboard(const MemStats *mem, const CPUInfo *info, const CPUUsage *usage) {
    // Recalcular layout si cambió tamaño
    ensure_layout();

    // -------- Encabezado en stdscr (no borramos todo) --------
    int rows, cols; getmaxyx(stdscr, rows, cols);
    const char *title = "Raspberry Pi — System Info (TUI)";
    const char *subtitle = "[Dashboard] Memoria instalada + uso RAM/SWAP + CPU (modelo, núcleos y carga por núcleo)";
    const char *hint = "Auto-refresh 2 s — 'r' refrescar, 'q' salir";

    // Limpiamos solo cabecera previa
    mvhline(1, 0, ' ', cols);
    mvhline(3, 0, ' ', cols);

    int title_x = (cols - (int)strlen(title)) / 2;
    int sub_x   = (cols - (int)strlen(subtitle)) / 2;

    if (has_colors()) attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, title_x > 0 ? title_x : 0, "%s", title);
    if (has_colors()) attroff(COLOR_PAIR(1) | A_BOLD);

    if (has_colors()) attron(COLOR_PAIR(2));
    mvprintw(3, sub_x > 0 ? sub_x : 0, "%s", subtitle);
    if (has_colors()) attroff(COLOR_PAIR(2));

    // Timestamp
    time_t now = time(NULL); struct tm *tm = localtime(&now);
    char ts[32]; strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);
    mvprintw(1, cols - (int)strlen(ts) - 2, "%s", ts);

    // -------- Si no hay ventanas (terminal pequeña) → fallback en stdscr --------
    if (!g_win_mem || !g_win_cpu) {
        int y = 5;
        // Limpia zona de contenido hasta el pie
        for (int r = y; r < rows - 2; ++r) mvhline(r, 0, ' ', cols);

        char installed[64], t_total[64], t_used[64], t_avail[64], s_total[64], s_used[64], s_free[64];
        format_bytes_from_kib(mem->total_kib,      installed, sizeof(installed));
        format_bytes_from_kib(mem->total_kib,      t_total,   sizeof(t_total));
        format_bytes_from_kib(mem->used_kib,       t_used,    sizeof(t_used));
        format_bytes_from_kib(mem->avail_kib,      t_avail,   sizeof(t_avail));
        format_bytes_from_kib(mem->swap_total_kib, s_total,   sizeof(s_total));
        format_bytes_from_kib(mem->swap_used_kib,  s_used,    sizeof(s_used));
        format_bytes_from_kib(mem->swap_free_kib,  s_free,    sizeof(s_free));

        mvprintw(y++, 2, "Instalada: %s", installed);
        mvprintw(y++, 2, "RAM:  total %s | usada %s | disp. %s | uso %.1f%%",
                 t_total, t_used, t_avail, mem->used_ratio * 100.0);
        draw_bar(y++, 2, cols - 4, mem->used_ratio);
        mvprintw(y++, 2, "SWAP: total %s | usada %s | libre %s | uso %.1f%%",
                 s_total, s_used, s_free,
                 (mem->swap_total_kib > 0 ? mem->swap_used_ratio * 100.0 : 0.0));
        draw_bar(y++, 2, cols - 4, (mem->swap_total_kib > 0) ? mem->swap_used_ratio : 0.0);

        mvprintw(y++, 2, "CPU: %s  (SoC: %s, Arch: %s)", info->model, info->hardware, info->arch);
        mvprintw(y++, 2, "Núcleos: en línea %d / configurados %d", info->cores_online, info->cores_config);
        if (!usage->ready) {
            mvprintw(y++, 2, "Cargas por núcleo: calculando…");
        } else {
            mvprintw(y++, 2, "Cargas por núcleo (promedio: %.1f%%):", usage->avg * 100.0);
            for (int i = 0; i < usage->cores && y < rows - 3; ++i) {
                char label[32];
                snprintf(label, sizeof(label), "core %d: %5.1f%%", i, usage->per_core[i] * 100.0);
                mvprintw(y, 2, "%s", label);
                int bx = 2 + (int)strlen(label) + 2;
                draw_bar(y, bx, cols - bx - 2, usage->per_core[i]);
                y++;
            }
        }

        mvprintw(rows - 2, 2, "%s", hint);
        // Un solo “blit”
        wnoutrefresh(stdscr); doupdate();
        return;
    }

    // -------- Con ventanas persistentes (sin flicker) --------
    // Limpia solo el interior de cada panel
    werase(g_win_mem); werase(g_win_cpu);
    box(g_win_mem, 0, 0); box(g_win_cpu, 0, 0);
    if (has_colors()) wattron(g_win_mem, COLOR_PAIR(5) | A_BOLD);
    mvwprintw(g_win_mem, 0, 2, "[ Memoria ]");
    if (has_colors()) wattroff(g_win_mem, COLOR_PAIR(5) | A_BOLD);
    if (has_colors()) wattron(g_win_cpu, COLOR_PAIR(5) | A_BOLD);
    mvwprintw(g_win_cpu, 0, 2, "[ CPU ]");
    if (has_colors()) wattroff(g_win_cpu, COLOR_PAIR(5) | A_BOLD);

    // Dimensiones útiles
    int wrows, wcols; getmaxyx(g_win_mem, wrows, wcols);
    int wy = 1;
    int inner_bar_w = wcols - 4 - 2; if (inner_bar_w < 10) inner_bar_w = 10;

    // --- Memoria ---
    char installed[64], t_total[64], t_used[64], t_avail[64], s_total[64], s_used[64], s_free[64];
    format_bytes_from_kib(mem->total_kib,      installed, sizeof(installed));
    format_bytes_from_kib(mem->total_kib,      t_total,   sizeof(t_total));
    format_bytes_from_kib(mem->used_kib,       t_used,    sizeof(t_used));
    format_bytes_from_kib(mem->avail_kib,      t_avail,   sizeof(t_avail));
    format_bytes_from_kib(mem->swap_total_kib, s_total,   sizeof(s_total));
    format_bytes_from_kib(mem->swap_used_kib,  s_used,    sizeof(s_used));
    format_bytes_from_kib(mem->swap_free_kib,  s_free,    sizeof(s_free));

    mvwprintw(g_win_mem, wy++, 2, "Instalada: %-10s", installed);
    mvwprintw(g_win_mem, wy++, 2, "RAM:   total %10s | usada %10s | disp. %10s | uso %5.1f%%",
              t_total, t_used, t_avail, mem->used_ratio * 100.0);
    draw_bar_win(g_win_mem, wy++, 2, inner_bar_w, mem->used_ratio);
    wy++;
    mvwprintw(g_win_mem, wy++, 2, "SWAP:  total %10s | usada %10s | libre %10s | uso %5.1f%%",
              s_total, s_used, s_free,
              (mem->swap_total_kib > 0 ? mem->swap_used_ratio * 100.0 : 0.0));
    draw_bar_win(g_win_mem, wy++, 2, inner_bar_w, (mem->swap_total_kib > 0) ? mem->swap_used_ratio : 0.0);

    // --- CPU ---
    getmaxyx(g_win_cpu, wrows, wcols);
    wy = 1;
    mvwprintw(g_win_cpu, wy++, 2, "CPU: %s  (SoC: %s, Arch: %s)", info->model, info->hardware, info->arch);
    mvwprintw(g_win_cpu, wy++, 2, "Núcleos: en línea %d / configurados %d", info->cores_online, info->cores_config);

    if (!usage->ready) {
        mvwprintw(g_win_cpu, wy++, 2, "Cargas por núcleo: calculando… (espera un ciclo)");
    } else {
        mvwprintw(g_win_cpu, wy++, 2, "Cargas por núcleo (promedio: %.1f%%):", usage->avg * 100.0);
        for (int i = 0; i < usage->cores && wy < wrows - 1; ++i) {
            char label[32];
            snprintf(label, sizeof(label), "core %d: %5.1f%%", i, usage->per_core[i] * 100.0);
            mvwprintw(g_win_cpu, wy, 2, "%s", label);
            int bx = 2 + (int)strlen(label) + 2;
            int bw = wcols - bx - 2; if (bw < 10) bw = 10;
            draw_bar_win(g_win_cpu, wy, bx, bw, usage->per_core[i]);
            wy++;
        }
    }

    // Pie de página
    mvhline(rows - 2, 0, ' ', cols);
    mvprintw(rows - 2, 2, "%s", hint);

    // Composición: todas las ventanas al “off-screen buffer”… y un solo “blit”
    wnoutrefresh(g_win_mem);
    wnoutrefresh(g_win_cpu);
    wnoutrefresh(stdscr);
    doupdate();
}


// ==================== Entrada con timeout ====================

int tui_getch(void) { return getch(); }
int tui_was_timeout(int ch) { return (ch == ERR); }
void tui_set_timeout_ms(int ms) { timeout(ms); }

// ==================== Helpers de barra ====================

// Versión que escribe en stdscr (usada por vistas antiguas)
static void draw_bar(int y, int x, int width, double ratio) {
    if (width <= 4) return;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    int fill = (int)(ratio * (double)(width - 2) + 0.5);
    if (fill < 0) fill = 0;
    if (fill > width - 2) fill = width - 2;

    mvprintw(y, x, "[");
    mvprintw(y, x + width - 1, "]");

    short pair = 3; // verde
    if (ratio >= 0.90) pair = 4;        // rojo
    else if (ratio >= 0.70) pair = 2;   // amarillo

    if (has_colors()) attron(COLOR_PAIR(pair) | A_BOLD);
    for (int i = 0; i < fill; ++i) mvprintw(y, x + 1 + i, "#");
    if (has_colors()) attroff(COLOR_PAIR(pair) | A_BOLD);

    for (int i = fill; i < width - 2; ++i) mvprintw(y, x + 1 + i, " ");
}

// Versión que escribe en una ventana (panel)
static void draw_bar_win(WINDOW *w, int y, int x, int width, double ratio) {
    if (!w || width <= 4) return;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    int fill = (int)(ratio * (double)(width - 2) + 0.5);
    if (fill < 0) fill = 0;
    if (fill > width - 2) fill = width - 2;

    mvwprintw(w, y, x, "[");
    mvwprintw(w, y, x + width - 1, "]");

    short pair = 3; // verde
    if (ratio >= 0.90) pair = 4;        // rojo
    else if (ratio >= 0.70) pair = 2;   // amarillo

    if (has_colors()) wattron(w, COLOR_PAIR(pair) | A_BOLD);
    for (int i = 0; i < fill; ++i) mvwprintw(w, y, x + 1 + i, "#");
    if (has_colors()) wattroff(w, COLOR_PAIR(pair) | A_BOLD);

    for (int i = fill; i < width - 2; ++i) mvwprintw(w, y, x + 1 + i, " ");
}

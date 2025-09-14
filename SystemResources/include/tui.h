#ifndef TUI_H
#define TUI_H

#include "memory.h"
#include "cpu.h"

// Inicializa la TUI (ncurses). Retorna 0 si OK, !=0 si falla.
int tui_init(void);

// Termina la TUI limpiamente.
void tui_end(void);

// Dibuja la pantalla con la memoria instalada (texto centrado).
// 'installed_str' es una cadena preformateada (ej. "0.98 GiB").
void tui_draw_memory(const char *installed_str);

// pantalla con barras de uso de RAM y SWAP
void tui_draw_memory_usage(const MemStats *st);

// dashboard completo (act. 1–5 en la misma pantalla)
void tui_draw_system_dashboard(const MemStats *mem, const CPUInfo *info, const CPUUsage *usage);

// Lee una tecla (o timeout) desde la TUI.
// Devuelve un código de tecla, o un valor especial que puedes preguntar con tui_was_timeout().
int tui_getch(void);

// Devuelve 1 si 'ch' representa que se cumplió el timeout (no se presionó ninguna tecla).
int tui_was_timeout(int ch);

// Cambia el timeout en milisegundos (equivalente a timeout(ms) de ncurses).
void tui_set_timeout_ms(int ms);

// Dashboard con Actividad 1 + 2 en la misma pantalla
void tui_draw_memory_dashboard(const MemStats *st);


#endif // TUI_H

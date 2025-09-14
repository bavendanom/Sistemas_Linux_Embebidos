# rpi-sysinfo-tui
Aplicación TUI en C para mostrar especificaciones de una Raspberry Pi (memoria, CPU, núcleos, carga por core, etc.).  
**Actividad 1:** Memoria instalada.

## Build
```bash
sudo apt install -y build-essential libncursesw5-dev
make           # modo debug por defecto
make run       # ejecuta
make release   # compila optimizado (O2)

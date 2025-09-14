# -------- Configuración general --------
APP      := rpi-sysinfo-tui
CC       := gcc
CSTD     := -std=c11
WARN     := -Wall -Wextra -Wconversion -Wshadow -Wpedantic
OPT_DBG  := -O0 -g3
OPT_REL  := -O2

INCDIR   := include
SRCDIR   := src
BUILDDIR := build

# Añade aquí nuevos .c cuando implementemos CPU, load, etc.
SRC := $(SRCDIR)/main.c \
       $(SRCDIR)/tui.c  \
       $(SRCDIR)/memory.c\
       $(SRCDIR)/cpu.c

OBJ := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC))

# ncurses para TUI
LIBS := -lncursesw
INCS := -I$(INCDIR)

# Perfil por defecto (debug). Para release: `make clean all MODE=release`
MODE ?= debug
ifeq ($(MODE),release)
  CFLAGS := $(CSTD) $(WARN) $(OPT_REL) $(INCS)
else
  CFLAGS := $(CSTD) $(WARN) $(OPT_DBG) $(INCS)
endif

# -------- Reglas principales --------
all: $(BUILDDIR)/$(APP)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(APP): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LIBS)

run: all
	./$(BUILDDIR)/$(APP)

clean:
	rm -rf $(BUILDDIR)

# Conveniencia
debug:
	$(MAKE) clean all MODE=debug

release:
	$(MAKE) clean all MODE=release

CC = gcc
CFLAGS = -O3 -march=native -std=c11 -Iinclude -Wall -Wextra -Wpedantic
PYTHON_INCLUDE := $(shell python3 -c "from sysconfig import get_paths; print(get_paths()['include'])")
PYTHON_LIB := $(shell python3-config --ldflags)

SRCDIR = src
PYBINDDIR = pythonBindings
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
PY_SOURCES = $(wildcard $(PYBINDDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
PY_OBJECTS = $(patsubst $(PYBINDDIR)/%.c,$(OBJDIR)/%.o,$(PY_SOURCES))

TARGET = $(BINDIR)/waao
PY_MODULE = waao$(shell python3-config --extension-suffix)

all: dirs $(TARGET) $(PY_MODULE)

dirs:
	mkdir -p $(OBJDIR) $(BINDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(PYBINDDIR)/%.c
	$(CC) $(CFLAGS) -I$(PYTHON_INCLUDE) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

$(PY_MODULE): $(OBJECTS) $(PY_OBJECTS)
	$(CC) -shared $(OBJECTS) $(PY_OBJECTS) -o $(PY_MODULE) $(PYTHON_LIB)

clean:
	rm -rf $(OBJDIR) $(BINDIR) *.so *.pyd

.PHONY: all clean dirs

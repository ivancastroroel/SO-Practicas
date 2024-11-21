# Variables
CC = gcc
CFLAGS = -Wall -g
SOURCES = p2.c list.c memoria.c listaMemoria.c listaProcesos.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = p2

# Regla principal
all: $(EXECUTABLE)

# Crear el ejecutable a partir de los objetos
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Compilar cada archivo .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar los archivos generados
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
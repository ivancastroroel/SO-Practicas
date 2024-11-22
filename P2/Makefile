# Variables
CC = gcc
CFLAGS = -Wall -g
SOURCES = p2.c lista.c memory.c memoryList.c
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
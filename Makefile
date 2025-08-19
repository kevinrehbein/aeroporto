CC = gcc

CFLAGS = -pthread

TARGET = simulador_aeroporto

SOURCES = aeroporto.c aviao.c main.c

OBJECTS = aeroporto.o aviao.o main.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Regra para compilar arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
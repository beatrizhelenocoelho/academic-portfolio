# nome do executável
EXEC = ndn

# compilador e opções
CC = gcc
CFLAGS = -Wall

# arquivo fonte
SRC = ndn.c

# Arquivos objeto
OBJ = $(SRC:.c=.o)

# Regra para compilar o projeto
all: $(EXEC)

# Regra para gerar o executável
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

# Regra para compilar os arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -f $(OBJ) $(EXEC)

# Regra para compilar com o comando 'make clean' antes de compilar
rebuild: clean all

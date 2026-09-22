# Makefile de exemplo

# compilador a utilizar
CC= gcc
# flags para compilar
CFLAGS= -Wall -g
# flags para linkar, incluíndo a biblioteca matemática
LDFLAGS= -lm
# gerar variáveis com listas de ficheiros de cada tipo para usar nas várias regras
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))
HEADERS=$(wildcard *.h)
PDFS=$(wildcard *.pdf)
TXTS=$(wildcard *.txt)

# nome do executável a produzir, compilar com "make"
TARGET= CNAES

all: $(TARGET)

# compilar executável com optimização, dar o comando "make release"
release: CFLAGS+= -O3
release: clean
release: $(TARGET)


$(TARGET): $(OBJS) Makefile
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

# listar dependências. para simplificar, cada ficheiro .o depende do correspondente .c, de todos os ficheiros .h e da Makefile
%.o: %.c $(HEADERS) Makefile
	$(CC) -c $(CFLAGS) -o $@ $<

# para criar um zip com todos os ficheiros modificados, dar o comando "make zip"
zip: $(TARGET).zip

$(TARGET).zip: Makefile $(SRCS) $(HEADERS) $(PDFS) $(TXTS)
	zip -9 $@ $?

# para criar um tar.gz com todos os ficheiros, dar o comando "make tgz"
tgz: $(TARGET).tar.gz

$(TARGET).tar.gz: Makefile $(SRCS) $(HEADERS) $(PDFS) $(TXTS)
	tar czvf $@ $^

# declaramos targets como PHONY quando não criam um ficheiro com o nome do target
.PHONY: all release clean zip tgz
# para forçar recompilação, dar o comando "make clean" e depois "make" ou o comando "make clean ; make"
clean:
	rm -f *.o $(TARGET)


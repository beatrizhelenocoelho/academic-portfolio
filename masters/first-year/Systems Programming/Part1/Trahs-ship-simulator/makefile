# Compiler and Flags
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lconfig++ -lSDL2 -lSDL2_gfx -lSDL2_ttf -lprotobuf-c -lzmq

# Executables
CLIENT = universe-client
SERVER = universe-server

# All source files
SRCS = $(wildcard *.c)

# Exclude primary main files from shared build
SHARED_SRCS = $(filter-out Trash-ship-simulator-client.c Trash-ship-simulator-server.c, $(SRCS))

# Object files
SHARED_OBJS = $(SHARED_SRCS:.c=.o)
CLIENT_OBJS = Trash-ship-simulator-client.o $(SHARED_OBJS)
SERVER_OBJS = Trash-ship-simulator-server.o $(SHARED_OBJS)

# Default target: build both
all: $(CLIENT) $(SERVER)

# Build client executable
$(CLIENT): $(CLIENT_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Build server executable
$(SERVER): $(SERVER_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Generic compilation rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Proto rule
proto: message_comms.proto
	protoc-c --c_out=. message_comms.proto

# Cleanup
clean:
	rm -f *.o

fclean: clean
	rm -f $(CLIENT) $(SERVER)

rebuild: fclean all

.PHONY: all clean fclean rebuild proto


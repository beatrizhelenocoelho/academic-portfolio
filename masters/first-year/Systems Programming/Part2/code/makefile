# =========================
# Compiler and Flags
# =========================
CC = gcc
CFLAGS = -Wall -g -Icommon_files -Iserver_only -Itrash_ship_only
LDFLAGS = -lm -lconfig -lSDL2 -lSDL2_gfx -lSDL2_ttf -lprotobuf-c -lzmq

# =========================
# Directories and files
# =========================
PROTO_DIR = common_files
PROTO = message_comms.proto
PY_DASH = dash_python

PROTO_C = $(PROTO_DIR)/message_comms.pb-c.c
PROTO_H = $(PROTO_DIR)/message_comms.pb-c.h

# =========================
# Executables
# =========================
CLIENT = new-universe-client
SERVER = new-universe-server

CLIENT_MAIN = new-trash-ship-client.c
SERVER_MAIN = new-universe-server.c

# =========================
# Common sources (including protobuf C files)
# =========================
COMMON_SRC = $(wildcard $(PROTO_DIR)/*.c) $(PROTO_C)
COMMON_OBJ = $(COMMON_SRC:.c=.o)

# =========================
# Server sources
# =========================
SERVER_DIR = server_only
SERVER_SRC = $(wildcard $(SERVER_DIR)/*.c)
SERVER_OBJ_SRC = $(SERVER_SRC:.c=.o)

# =========================
# Client sources
# =========================
SHIP_DIR = trash_ship_only
SHIP_SRC = $(wildcard $(SHIP_DIR)/*.c)
SHIP_OBJ = $(SHIP_SRC:.c=.o)

# =========================
# Final object lists
# =========================
CLIENT_OBJ = $(CLIENT_MAIN:.c=.o) $(COMMON_OBJ) $(SHIP_OBJ)
SERVER_OBJ_FINAL = $(SERVER_MAIN:.c=.o) $(COMMON_OBJ) $(SERVER_OBJ_SRC)

# =========================
# Targets
# =========================
all: proto $(CLIENT) $(SERVER)

# ----- Client -----
$(CLIENT): $(CLIENT_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

# ----- Server -----
$(SERVER): $(SERVER_OBJ_FINAL)
	$(CC) $^ -o $@ $(LDFLAGS)

# =========================
# Compilation rules
# =========================
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# =========================
# Protobuf generation
# =========================
proto: $(PROTO_C)

$(PROTO_C) $(PROTO_H): $(PROTO)
	protoc-c --c_out=$(PROTO_DIR) $<
	protoc --python_out=$(PY_DASH) $<

# =========================
# Cleaning
# =========================
clean:
	rm -f *.o $(COMMON_OBJ) $(SERVER_OBJ_SRC) $(SHIP_OBJ)

fclean: clean
	rm -f $(CLIENT) $(SERVER)

rebuild: fclean all

.PHONY: all clean fclean rebuild proto


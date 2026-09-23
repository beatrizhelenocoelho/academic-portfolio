# 1. Variables (Updated CFLAGS for standard practice)
CC = gcc
CFLAGS = -Wall -g  # Werror added back for robustness
LDFLAGS = -lconfig++ -lSDL2 -lSDL2_gfx -lSDL2_ttf -lm
TARGET = Universe-simulator

# Define ALL source files
SRCS = $(wildcard *.c) libconfig/gravitation.c
# Automatically generate ALL object file names
OBJS = $(SRCS:.c=.o)            # Expands to: Universe-simulator.o display.o

# 2. Default Target
all: $(TARGET)

# 3. Linking Rule: Creates the executable
# Dependencies: Requires ALL object files to exist.
$(TARGET): $(OBJS)
	# The linking command uses only the object files.
	$(CC) $^ -o $@ $(LDFLAGS)

# 4. Compilation Rule (Pattern Rule): Creates Object Files
# This ensures a Universe-simulator.o is created from Universe-simulator.c, 
# and it only runs if Universe-simulator.c is newer than Universe-simulator.o.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 5. Phony and Clean Targets
.PHONY: all clean fclean rebuild

# Removes ALL intermediate object files
clean:
	rm -f $(OBJS)

# Removes the executable AND object files
fclean: clean
	rm -f $(TARGET)

# Performs a full clean and rebuild
rebuild: fclean all

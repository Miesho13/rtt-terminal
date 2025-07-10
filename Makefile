# --- CONFIG ---
CC = gcc
CFLAGS = -Wall -O0 -g
LDFLAGS =

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
OBJDIR = build
OBJPATHS = $(addprefix $(OBJDIR)/, $(OBJ))

TARGET = $(OBJDIR)/rtt-terminal

# --- RULES ---
all: $(TARGET)

# Compile .c to .o
$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link to final binary
$(TARGET): $(OBJPATHS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean build files
clean:
	rm -rf $(OBJDIR)

.PHONY: all clean run


# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2 -I$(INCDIR) -MMD -MP

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = build
BINDIR = bin

# Sources and objects
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)

# Target executable
TARGET = $(BINDIR)/http-server

# Default build
all: $(TARGET)

# Link executable
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build and bin directories exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Include dependencies
-include $(DEPS)

# Run server
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all clean run

CC = gcc
CFLAGS = -Wall -g
LIBS = -lGL -lGLEW -lGLFW

SRC = src/main.c src/shader_utils.c
OBJ = $(SRC:.c=.o)
EXEC = shader_cli

all: $(EXEC)

$(EXEC): $(OBJ)
    $(CC) $(OBJ) -o $(EXEC) $(LIBS)

.c.o:
    $(CC) $(CFLAGS) -c $< -

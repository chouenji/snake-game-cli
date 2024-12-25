CC = gcc
CFLAGS = -Wall -Wextra -Werror

SRC = main.c
TARGET = main

all:
	@$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	@./$(TARGET)

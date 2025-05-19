.PHONY: run clean

CC = gcc
FLAG = -o
SRC = legend_of_zelda.c
TARGET = legend_of_zelda

$(TARGET): $(SRC)
	$(CC) $(SRC) $(FLAG) $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
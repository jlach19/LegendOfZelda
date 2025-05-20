.PHONY: run clean

CC = gcc
FLAG = -o
SRC = zelda_rogue.c
TARGET = zelda_rogue

$(TARGET): $(SRC)
	$(CC) $(SRC) $(FLAG) $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
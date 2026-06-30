CC     = gcc
TARGET = 3d_numbers
SRC    = 3d_numbers.c

CFLAGS  = -O2 -Wall -std=c99
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean

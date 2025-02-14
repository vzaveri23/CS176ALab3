CC = gcc
CFLAGS = -Wall -Wextra
TARGET = PingClient
SRC = PingClient.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lm

clean:
	rm -f $(TARGET)

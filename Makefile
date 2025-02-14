CC = gcc
CFLAGS = -Wall -Wextra 
TARGET = PingClient
SRC = PingClient.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

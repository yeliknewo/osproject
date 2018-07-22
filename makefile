OUT=server
SOURCES=main.cpp
CC=g++
CFLAGS=-g -Wall -std=c++17 -pthread
LDFLAGS=

all: clean $(SOURCES) $(OUT)

$(OUT): $(OBJECTS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

clean:
	rm -f *.o $(OUT)
	sudo pkill -f $(OUT)
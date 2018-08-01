OUT=server_os_project
SOURCES=main.cpp
CC=g++
CFLAGS=-g -Wall -std=c++17 -pthread
LDFLAGS=
PORT=8080
FILES=./files

all: clean $(SOURCES) $(OUT)

$(OUT): $(OBJECTS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

clean:
	rm -f *.o $(OUT)
	sudo pkill -f $(OUT)

run:
	./$(OUT) $(PORT) $(FILES)
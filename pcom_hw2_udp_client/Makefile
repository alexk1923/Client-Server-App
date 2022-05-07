# Protocoale de comunicatii:
# Laborator 8: Multiplexare
# Makefile

CFLAGS = -Wall -g

# Portul pe care asculta serverul (de completat)
PORT = 2000
ID = "Lala"

# Adresa IP a serverului (de completat)
IP_SERVER = 127.0.0.1

all: server subscriber

# Compileaza server.c
server: server.cpp
# Compileaza client.c
subscriber: subscriber.cpp


.PHONY: clean run_server run_subscriber

# Ruleaza serverul
run_server: server
	./server ${PORT}

# Ruleaza clientul
run_subscriber: subscriber
	./subscriber ${ID} ${IP_SERVER} ${PORT}

clean:
	rm -f server client

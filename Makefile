CFLAGS = -g -I /usr/include/tirpc -Wall -Wno-register -Wno-unused-variable
LDFLAGS = -lnsl -ltirpc
CC = g++

all: client server

client: tema1_client.cpp tema1_clnt.cpp tema1_xdr.cpp token.cpp
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


server: tema1_server.cpp tema1_svc.cpp tema1_xdr.cpp token.cpp
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o client server
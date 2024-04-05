CLIENT=client
SERVER=server
CC=g++
INCLUDE_PATH=../include
CFLAGS=-I$(INCLUDE_PATH)
install:
	$(CC) $(CLIENT).cpp -o $(CLIENT) $(CFLAGS)
	mv $(CLIENT) ../
	$(CC) $(SERVER).cpp -o $(SERVER) $(CFLAGS)
	mv $(SERVER) ../
clean:
	rm -rf ../$(CLIENT) ../$(SERVER)
	rm -rf ../pubpipe
all: bin/ftp

bin/client/client.o: client/client.c
	gcc -g -Wall -Werror -std=c99 -Iinclude client/client.c -c -o bin/client.o

bin/server/server.o: server/server.c
	gcc -g -Wall -Werror -std=c99 -Iinclude server/server.c -c -o bin/server.o

lib/ftp.a:
	ar rcs lib/ftp.a

bin/ftp: lib/ftp.a server/server.c client/client.c
	gcc -L lib/ftp.a server/server.c -o bin/server/server.o
	gcc -L lib/ftp.a client/client.c -o bin/client/client.o

clean:
	rm bin/server/*.o
	rm bin/client/*.o

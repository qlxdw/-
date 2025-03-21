
all:client server

CFLAGS=-g -Wall

client:client.c net.c file_transfer.c

server:server.c net.c file_transfer.c

install:
	cp client client-mod/
	cp server server-mod/
#cp server /mnt/hgfs/download-share/

clean:
	-rm client server

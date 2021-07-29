CFLAGS=-c
CC=gcc

PROG = command.o update-cache.o read-cache.o show-diff.o init-db.o cat-file.o commit-tree.o write-tree.o commit.o read-tree.o

LIBS= -lssl -lz -lcrypto

git: $(PROG) 
	$(CC) $(PROG) -o git $(LIBS)

command.o: command.c 
	$(CC) $(CFLAGS) command.c -o command.o $(LIBS)
	
update-cache.o: update-cache.c 
	$(CC) $(CFLAGS) update-cache.c -o update-cache.o $(LIBS)

show-diff.o: show-diff.c
	$(CC) $(CFLAGS) show-diff.c -o show-diff.o $(LIBS)

init-db.o: init-db.c 
	$(CC) $(CFLAGS) init-db.c -o init-db.o $(LIBS)

write-tree.o: write-tree.c 
	$(CC) $(CFLAGS) write-tree.c -o write-tree.o $(LIBS)

commit-tree.o: commit-tree.c 
	$(CC) $(CFLAGS) commit-tree.c -o commit-tree.o $(LIBS)

cat-file.o: cat-file.c 
	$(CC) $(CFLAGS) cat-file.c -o cat-file.o $(LIBS)

commit.o: commit.c 
	$(CC) $(CFLAGS) commit.c -o commit.o $(LIBS)

read-cache.o: read-cache.c
	$(CC) $(CFLAGS) read-cache.c -o read-cache.o $(LIBS)

read-tree.o: read-tree.c
	$(CC) $(CFLAGS) read-tree.c -o read-tree.o $(LIBS)
 
.PHONY:
clear: 
	rm -rf $(PROG)
clearall:
	rm -rf $(PROG) git
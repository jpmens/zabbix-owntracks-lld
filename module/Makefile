CFLAGS	+=-Wall -Werror -I/usr/local/include
LIBS 	+= -L/usr/local/lib -llmdb

TARGETS= main
OBJS = ldb.o json.o



main: main.c ldb.h $(OBJS) Makefile
	$(CC) -DTESTING $(CFLAGS) -o main main.c $(OBJS) $(LIBS)

ldb.o: ldb.c ldb.h json.h

clean:
	rm -f *.o
clobber: clean
	rm -f main

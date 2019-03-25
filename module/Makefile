CFLAGS	+=-Wall -Werror -I/usr/local/include
LIBS 	+= -L/usr/local/lib -llmdb

TARGETS= main
OBJS = ldb.o json.o 


main: main.o $(OBJS) 
	$(CC) $(CFLAGS) -o main main.o $(OBJS) $(LIBS)

main.o: main.c ldb.c ldb.h json.h Makefile 

clean:
	rm -f *.o
clobber: clean
	rm -f main

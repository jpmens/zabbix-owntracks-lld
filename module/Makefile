CFLAGS	+=-Wall -Werror -I/usr/local/include
#	-I../../zabbix-4.0.5/include/
LIBS 	+= -L/usr/local/lib -llmdb

TARGETS= main
SRC = owntracks-module.c
SONAME = owntracks-module.so
OBJS = ldb.o json.o

all: main $(SONAME)

main: $(SRC) ldb.h $(OBJS) Makefile
	$(CC) -DTESTING $(CFLAGS) -o main $(SRC) $(OBJS) $(LIBS)

$(SONAME): $(SRC) $(OBJS) Makefile
	gcc -fPIC -shared $(CFLAGS) -o $(SONAME) $(SRC) $(OBJS) $(LIBS)
	install $(SONAME) /usr/local/zabbix/lib/modules/$(SONAME)

ldb.o: ldb.c ldb.h json.h

clean:
	rm -f *.o
clobber: clean
	rm -f main $(SONAME)

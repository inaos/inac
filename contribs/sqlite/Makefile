SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o) $(DIRS:%=%.o)
THIS_LIB = sqlite.a


all: $(OBJS)
	@ar rcs $(THIS_LIB) $(OBJS)
	gcc -shared -o libsqlite3.so -fPIC sqlite3.o -ldl -lpthread

%.o: %.c
	@echo compile $(CC) $<
	@$(CC) $(CFLAGS) -fPIC -c $<

.PHONY: clean

clean:
	@-rm -f *.o

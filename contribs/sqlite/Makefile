SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o) $(DIRS:%=%.o)
LIB = sqlite.a
CLI = sqlite


$(CLI): $(LIB)
	$(CC) $(CFLAGS) -o $(CLI) shell.o $(LIB)  $(LDFLAGS) -lpthread -ldl

$(LIB): $(OBJS)
	@ar rcs $(LIB) $(OBJS)

%.o: %.c
	@echo compile $(CC) $<
	@$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	@-rm -f *.o

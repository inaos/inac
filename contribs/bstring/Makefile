SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o) $(DIRS:%=%.o)
THIS_LIB = bstring.a


all: $(OBJS)
	@ar rcs $(THIS_LIB) $(OBJS)

%.o: %.c
	@echo compile $(CC) $<
	@$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	@-rm -f *.o

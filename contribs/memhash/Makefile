SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o) $(DIRS:%=%.o)
THIS_LIB = memhash.a

MACHINE := $(shell uname -m)
OS := $(shell uname -s)

all: $(OBJS)
	@echo assambling for: $(OS) $(MACHINE)
ifeq ($(MACHINE), x86_64)
	@echo assambling memhash.s
ifeq ($(OS), Darwin)
	@gcc -g -c llvm/x86_64/memhash.s -o memhash.o
else
	@gcc -g -c gas/x86_64/memhash.s -o memhash.o
endif
else
	@echo assambling memhash.s
	@gcc -m32 -g -c gas/i686/memhash.s -o memhash.o
endif
	@ar rcs $(THIS_LIB) $(OBJS) memhash.o

%.o: %.c
	@echo compile $(CC) $<
	@$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	@-rm -f *.o *.a

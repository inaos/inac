SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o) $(DIRS:%=%.o)
THIS_LIB = cpu-topology.a

MACHINE := $(shell uname -m)

all: $(OBJS)
	@echo assambling for: $(MACHINE)
ifeq ($(MACHINE), x86_64)
	@echo assambling get_cpuid_lix64.s
	@gcc -g -c get_cpuid_lix64.s -o get_cpuid_lix64.o
else
	@echo assambling get_cpuid_lix32.s
	@gcc -m32 -g -c get_cpuid_lix32.s -o get_cpuid_lix32.o
endif
	@ar rcs $(THIS_LIB) $(OBJS) get_cpuid*.o

%.o: %.c
	@echo compile $(CC) $<
	@$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	@-rm -f *.o *.a

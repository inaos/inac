DIRS = $(shell ls -p |grep /)
SRC = $(wildcard test*.c)
OBJ = $(SRC:%.c=%.o)
HDR = $(SRC:%.c=%.h)
FN  = $(SRC:%.c=%)
BIN = test

MAKEHEADERS = ../contribs/makeheaders/makeheaders
CFLAGS = -Wall -std=c89 -g -I../include -I../ -DINA_LIB -DDEBUG -DTRACE_ENABLED


all: $(OBJ) suites.o main.o
	@for i in $(DIRS); do $(MAKE) -C $$i; done
	@$(CC) $(CFLAGS) -o $(BIN) main.o suites.o $(OBJ) $(LDFLAGS) -undefined dynamic_lookup -pagezero_size 10000 -image_base 100000000 ../src/libinac.a ../contribs/anet/anet.a ../contribs/skiplist/skiplist.a ../contribs/luajit/src/libluajit.a

%.o: %.c
	@echo compiling $<
	@$(CC) $(CFLAGS) -I../include -I../ -c $<
	
.PHONY: clean

suites.o: $(MAKEHADERS)
	@echo making test headers...
	@echo "#ifndef _SUITES_H_" > suites.h
	@echo "#define _SUITES_H_" >> suites.h
	@echo "#include \"suites.h\"" > suites.c
	@echo "void runtests(const char* pattern) {" >> suites.c
	@for i in $(FN); do \
		$(MAKEHEADERS) $$i.c; \
		cat $$i.h | sed '1d' | sed -e 's/^ *[^ ]* //' >> suites.c; \
		echo "#include \""$$i.h\" >> suites.h; \
	done
	@echo "}" >> suites.c;
	@echo "void runtests(const char* pattern);" >> suites.h
	@echo "#endif" >> suites.h
	@$(CC) $(CFLAGS) -c suites.c
		
clean:
	@for i in $(DIRS); do $(MAKE) clean -C $$i; done
	@rm -f *.o test suites.* $(HDR)

test: all
	@for i in $(DIRS); do $(MAKE) test -C $$i; done
	./test

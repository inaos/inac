DIRS = $(shell ls -p |grep /)
SRC = $(wildcard test*.c)
OBJ = $(SRC:%.c=%.o)
HDR = $(SRC:%.c=%.h)
FN  = $(SRC:%.c=%)
BIN = test
MAKEHEADERS = ../contribs/makeheaders/makeheaders
CFLAGS = -Wall -std=c89 -g -I../include -DINA_LIB -DDEBUG -DTRACE_ENABLED


all: $(OBJ) suites.o main.o 
	for i in $(DIRS); do $(MAKE) -C $$i; done
	$(CC) $(CFLAGS) -v -o $(BIN) main.o suites.o $(OBJ) $(LDFLAGS) ../src/libinac.a
	
%.o: %.c
	$(CC) $(CFLAGS) -I../include -c $<
	
.PHONY: clean

suites.o: $(MAKEHADERS)
	/bin/echo "#ifndef _SUITES_H_" > suites.h
	/bin/echo "#define _SUITES_H_" >> suites.h
	/bin/echo "#include \"suites.h\"" > suites.c
	/bin/echo "void runtests() {" >> suites.c
	for i in $(FN); do \
		$(MAKEHEADERS) $$i.c; \
		cat $$i.h | sed '1d' | sed -e 's/^ *[^ ]* //' >> suites.c; \
		echo "#include \""$$i.h\" >> suites.h; \
	done
	/bin/echo "}" >> suites.c;
	/bin/echo "void runtests();" >> suites.h
	/bin/echo "#endif" >> suites.h
	$(CC) $(CFLAGS) -c suites.c
		
clean:
	for i in $(DIRS); do $(MAKE) clean -C $$i; done
	-rm -f *.o test suites.* $(HDR)

test: all
	for i in $(DIRS); do $(MAKE) test -C $$i; done
	./test

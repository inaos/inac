DIRS = $(shell ls -p |grep /)
SRC = $(wildcard test*.c)
OBJ = $(SRC:%.c=%.o)
HDR = $(SRC:%.c=%.h)
FN  = $(SRC:%.c=%)
BIN = test
MAKEHEADERS = $(INAC_CONTRIBS_DIR)/makeheaders/makeheaders

# ****************************************************************************
# Linker settings
# ****************************************************************************
LDFLAGS += ../src/$(INAC_LIB) $(INAC_LIBS)
ifeq (,$(findstring Windows,$(OS)))
	ifneq (Darwin,$(shell uname -s))
		LDFLAGS += -lm -lrt -ldl
	else
		LDFLAGS += -undefined dynamic_lookup -pagezero_size 10000 -image_base 100000000 
	endif
endif

all: $(OBJ) suites.o main.o
	@echo $(INAC_LIB)
	@$(CC) $(CFLAGS) -o $(BIN) main.o suites.o $(OBJ) $(LDFLAGS)

%.o: %.c
	@echo compiling $<
	@$(CC) $(CFLAGS)  -c $<

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
	@echo cleaning tests...
	@-rm -f *.o test suites.* $(HDR)

test: all
	./test

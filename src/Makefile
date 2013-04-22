#
# Copyright (c) 2013, INAOS GmbH
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the INAOS GmbH nor the names of its contributors
#       may be used to endorse or promote products derived from this software 
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
# ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
DIRS = $(shell ls -p |grep /)

SRC = $(wildcard *.c)
SRC_LUA = $(wildcard *.lua)

OBJS = $(SRC:%.c=%.o)
OBJS += $(SRC_LUA:%.lua=%.o)


all: $(OBJS) $(TARGET_LIB)
	@echo compiling all for $(TARGET_LIB) $(INC_LIBS)...
	@for i in $(DIRS); do $(MAKE) -C $$i; done

%.o: %.c
	@echo compile $(CC) $<
	@$(CC) $(CFLAGS) -c $<

%.o: %.lua
	@echo compile $(INAC_LUAJIT_CMD) $<
	@$(INAC_LUAJIT_CMD) $<  $(basename $<).o

libinac.a:
	@echo merge libraries... $(TARGET_LIB) $(INC_LIBS)
	@ar crs $(TARGET_LIB) $(OBJS) $(INC_LIBS)
	@for i in $(DIRS); do ar crs $(TARGET_LIB) $$i/*.o; done

.PHONY: clean

clean:
	@echo cleaning src...
	@for i in $(DIRS); do $(MAKE) clean -C $$i; done
	@-rm -f *.o *.a


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

# ****************************************************************************
# Set general Environment variables
# ****************************************************************************
INAC_HOME_DIR=$(CURDIR)
INAC_CONTRIBS_DIR=$(INAC_HOME_DIR)/contribs
export INAC_HOME_DIR
export INAC_CONTRIBS_DIR

# ****************************************************************************
# LuaJIT variables
# ****************************************************************************
INAC_LUAJIT_DIR=$(INAC_CONTRIBS_DIR)/luajit/src
INAC_LUAJIT_CMD=$(INAC_CONTRIBS_DIR)/luajit/src/luajit -b
INAC_LUA_PATH=$(INAC_LUAJIT_DIR)/?.lua
LUA_PATH=$(INAC_LUA_PATH)
export INAC_LUAJIT_DIR
export INAC_LUAJIT_CMD
export LUA_PATH

# ****************************************************************************
# Compiler setting
# ****************************************************************************
CC     = /usr/bin/gcc 
CFLAGS = -Wall -I$(INAC_HOME_DIR) -I$(INAC_HOME_DIR)/include \
         -I$(INAC_CONTRIBS_DIR)/bstring -I$(INAC_CONTRIBS_DIR)/sqlite \
         -I$(INAC_CONTRIBS_DIR)/skiplist -I$(INAC_CONTRIBS_DIR)/http-parser \
         -I$(INAC_CONTRIBS_DIR)/rapidxml -DINA_LIB=1
export CFLAGS
export LDFLAGS

# ****************************************************************************
# Subdirectories
# ****************************************************************************
DIRS = contribs doc include src tests
# ****************************************************************************
# Libraries
# ****************************************************************************
TARGET_LIB=libinac.a
INC_LIBS=$(INAC_CONTRIBS_DIR)/anet/anet.a $(INAC_CONTRIBS_DIR)/bstring/bstring.a \
     $(INAC_CONTRIBS_DIR)/luajit/src/libluajit.a $(INAC_CONTRIBS_DIR)/skiplist/skiplist.a \
     $(INAC_CONTRIBS_DIR)/sqlite/sqlite.a
export TARGET_LIB
export INC_LIBS

default: release

all: 
	@echo === INAOS Common C Library - $(INAC_BUILD_TYPE) -  ===
	@echo $(INAC_INCLUDES)
	@echo Building....
	@for i in $(DIRS); do $(MAKE) -C $$i; done
	@echo === Done ===
	@echo Architecture: $(OS)
	@echo Home directory: $(INAC_HOME_DIR)
	@echo LuaJIT command: $(INAC_LUAJIT_CMD)
	@echo Lua path: $(INAC_LUA_PATH)
	@echo Build type: $(INAC_BUILD_TYPE)

release: CFLAGS += -O2 -DINA_LOG_LEVEL=1
	export CFLAGS
release: INAC_BUILD_TYPE = release
	export INAC_BUILD_TYPE
release: all
	
debug: CFLAGS += -g -DDEBUG -DINA_TRACE_ENABLED=1 -DINA_TRACE_LEVEL=1 -DINA_LOG_LEVEL=4
	export CFLAGS
debug: INAC_BUILD_TYPE = debug
	export INAC_BUILD_TYPE
debug: all

.PHONY: clean

clean:
	@echo cleaning...
	@for i in $(DIRS); do $(MAKE) clean -C $$i; done
	@-rm -f ChangeLog

test: 
	$(MAKE) test -C tests


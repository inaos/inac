#
# Copyright (c) 2013-2014, INAOS GmbH
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
# Set make variables
# ****************************************************************************
OS := $(shell uname -s)

# ****************************************************************************
# Set general Environment variables
# ****************************************************************************
INAC_HOME_DIR=$(CURDIR)
INAC_CONTRIBS_DIR=$(INAC_HOME_DIR)/contribs
INAC_CONTRIBSBIN_DIR=$(INAC_HOME_DIR)/contribs-bin
export INAC_HOME_DIR
export INAC_CONTRIBS_DIR
export INAC_CONTRIBSBIN_DIR

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
CFLAGS = -Wall -I$(INAC_HOME_DIR) -I$(INAC_HOME_DIR)/include \
         -I$(INAC_CONTRIBS_DIR) -I$(INAC_CONTRIBSBIN_DIR)
CFLAGS += -DINA_LIB=1
CFLAGS += -freorder-blocks-and-partition

# ****************************************************************************
# Subdirectories
# ****************************************************************************
DIRS = contribs contribs-bin doc include src tests
# ****************************************************************************
# Libraries
# ****************************************************************************
INAC_LIB=libinac.a
ifeq ($(OS), Linux)
INAC_LINUX_LIBS=$(INAC_CONTRIBS_DIR)/cpu-topology/cpu-topology.a
endif
INAC_LIBS=$(INAC_CONTRIBS_DIR)/anet/anet.a \
	$(INAC_CONTRIBS_DIR)/luajit/src/libluajit.a $(INAC_CONTRIBS_DIR)/skiplist/skiplist.a \
	$(INAC_CONTRIBS_DIR)/sqlite/sqlite.a $(INAC_CONTRIBS_DIR)/rapidxml/rapidxml.a \
	$(INAC_CONTRIBS_DIR)/http-parser/libhttp_parser.a $(INAC_CONTRIBS_DIR)/axtls/axtls.a \
        $(INAC_CONTRIBS_DIR)/yajl/yajl.a $(INAC_CONTRIBS_DIR)/miniz/miniz.a \
	$(INAC_CONTRIBS_DIR)/lz4/lz4.a $(INAC_CONTRIBS_DIR)/timerwheel/timerwheel.a $(INAC_LINUX_LIBS)
# ****************************************************************************
#  String implementation
# ****************************************************************************
ifndef INAC_STRING_LIB
	INAC_STRING_LIB = istring
endif
ifeq (cstring,$(INAC_STRING_LIB))
	CFLAGS+=-DINA_CSTRING_ENABLED=1
endif
ifeq (istring,$(INAC_STRING_LIB))
	CFLAGS+=-DINA_ISTRING_ENABLED=1
endif
ifeq (bstring,$(INAC_STRING_LIB))
	INAC_LIBS+=$(INAC_CONTRIBS_DIR)/bstring/bstring.a
	CFLAGS+=-DINA_BSTRING_ENABLED=1
endif
ifeq (sds,$(INAC_STRING_LIB))
  	INAC_LIBS+=$(INAC_CONTRIBS_DIR)/sds/sds.a
	CFLAGS+=-DINA_SSTRING_ENABLED=1
endif
# ****************************************************************************
# Time implementation
# ****************************************************************************
ifeq (,$(INAC_TIME_BACKEND))
	INAC_TIME_BACKEND=os
endif
ifeq (meinberg, $(INAC_TIME_BACKEND))
	INAC_LIBS+=$(INAC_CONTRIBSBIN_DIR)/meinberg/lib64/mbgdevio.a
	CFLAGS+=-I$(INAC_CONTRIBSBIN_DIR)/meinberg
	CFLAGS+=-DINA_MBTIME_ENABLED=1
endif
ifeq (os, $(INAC_TIME_BACKEND))
	CFLAGS+=-DINA_OSTIME_ENABLED=1
endif
# ****************************************************************************
# Timer implementation
# ****************************************************************************
ifeq (,$(INAC_TIMER_BACKEND))
        INAC_TIMER_BACKEND=wheel
endif
ifeq (wheel, $(INAC_TIMER_BACKEND))
        CFLAGS+=-DINA_TIMER_BACKEND_WHEEL_ENABLED=1
endif
ifeq (skiplist, $(INAC_TIMER_BACKEND))
        CFLAGS+=-DINA_TIMER_SKIPLIST_ENABLED=1
endif
CFLAGS+=-DINA_STRING_DEFINED=1
CFLAGS+=-DINA_TIME_DEFINED=1
CFLAGS+=-DINA_TIMER_BACKEND_DEFINED=1
export CFLAGS
export LDFLAGS
export INAC_LIB
export INAC_LIBS
export INA_STRING_DEFINED
export INA_TIME_DEFINED
export INA_TIMER_BACKEND_DEFINED

default: release

all: 
	@echo === INAOS Common C Library - $(INAC_BUILD_TYPE) -  ===
	@echo Building....
	@echo "Architecture     : $(shell uname -p)"
	@echo "Build type       : $(INAC_BUILD_TYPE)"
	@echo "String library   : $(INAC_STRING_LIB)"
	@echo "Time backend     : $(INAC_TIME_BACKEND)"
	@echo "Timer backend    : $(INAC_TIMER_BACKEND)"
	@echo "CC               : $(CC)"
	@echo "CXX              : $(CXX)"
	@echo "GOV              : $(GCOV)"
	@echo "CFLAGS           : $(CFLAGS)"
	@echo "CXXFLAGS         : $(CXXFLAGS)"
	@echo "LDFLAGS          : $(LDFLAGS)"
	@echo "PATH             : $(PATH)"
	-rm -f src/$(INAC_LIB)
	@for i in $(DIRS); do $(MAKE) -C $$i $(INAC_BUILD_TYPE); done
	@echo === Done ===



release: CFLAGS += -O3 -flto -march=native -DINA_LOG_LEVEL=1
	export CFLAGS
release: INAC_BUILD_TYPE = release
	export INAC_BUILD_TYPE
release: all

debug: CFLAGS +=  -g -DDEBUG -DINA_TRACE_ENABLED=1 -DINA_TRACE_LEVEL=1 -DINA_LOG_LEVEL=4 
	export CFLAGS
debug: INAC_BUILD_TYPE = debug
	export INAC_BUILD_TYPE
debug: all

.PHONY: clean

clean:
	@echo cleaning...
	@for i in $(DIRS); do $(MAKE) clean -C $$i; done
	@-rm -f ChangeLog
	@-rm -f *.tap 

test: debug 
	$(MAKE) test -C tests

coverage-test: CFLAGS += --coverage
	export CFLAGS
coverage-test: LDFLAGS += --coverage -lgcov
	export LDFLAGS
coverage-test: test	

static-analyse: CFLAGS += -DINA_ASSERT_NOTNULL_ENABLED
	export CFLAGS
static-analyse: release

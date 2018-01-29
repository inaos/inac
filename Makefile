#
# Copyright (c) 2012-2018, INAOS GmbH
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
CFLAGS = -Wall -I$(INAC_HOME_DIR) -I$(INAC_HOME_DIR)/include \
         -I$(INAC_CONTRIBS_DIR)
CFLAGS += -DINA_LIB=1
ifneq (Darwin,$(shell uname -s))
CFLAGS += -fopenmp
endif
ifeq ($(OS), Linux)
CFLAGS += -freorder-blocks-and-partition
endif

# ****************************************************************************
# Subdirectories
# ****************************************************************************
DIRS = contribs doc include src tests tools
# ****************************************************************************
# Libraries
# ****************************************************************************
INAC_LIB=libinac.a
ifeq ($(OS), Linux)
INAC_LINUX_LIBS=$(INAC_CONTRIBS_DIR)/cpu-topology/cpu-topology.a
endif
INAC_LIBS=$(INAC_CONTRIBS_DIR)/anet/anet.a \
	$(INAC_CONTRIBS_DIR)/luajit/src/libluajit.a  \
	$(INAC_CONTRIBS_DIR)/miniz/miniz.a $(INAC_CONTRIBS_DIR)/xxhash/xxhash.a \
	$(INAC_CONTRIBS_DIR)/lz4/lz4.a $(INAC_CONTRIBS_DIR)/timerwheel/timerwheel.a \
	$(INAC_CONTRIBS_DIR)/falkhash/falkhash.a $(INAC_CONTRIBS_DIR)/memhash/memhash.a \
	$(INAC_LINUX_LIBS)
# ****************************************************************************
# Time implementation
# ****************************************************************************
CFLAGS+=-DINA_OSTIME_ENABLED=1
# ****************************************************************************
# Intel Compiler detection
# ****************************************************************************
INAC_INTEL_COMPILER = $(shell which icc >/dev/null; echo $$?)
ifeq "$(INAC_INTEL_COMPILER)" "0"
        CC = icc
	CXX = ipcp
	LDFLAGS += -parallel
        INAC_CC_DEBUG_FLAGS = -g -DDEBUG -march=core-avx2
        INAC_CC_RELEASE_FLAGS = -O3 -march=core-avx2
else
        INAC_CC_DEBUG_FLAGS = -g -DDEBUG -msse4.2 -maes
        INAC_CC_RELEASE_FLAGS = -O3 -flto -march=native -DINA_LOG_LEVEL=1
endif
CFLAGS+=-DINA_TIME_DEFINED=1
export CC
export CXX
export CFLAGS
export LDFLAGS
export INAC_LIB
export INAC_LIBS
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
	@echo "CFLAGS           : $(CFLAGS)"
	@echo "CXXFLAGS         : $(CXXFLAGS)"
	@echo "LDFLAGS          : $(LDFLAGS)"
	@echo "PATH             : $(PATH)"
	@echo "LD_LIBRARY_PATH  : $(LD_LIBRARY_PATH)"
	@echo "LIBRARY_PATH     : $(LIBRARY_PATH)"
	-rm -f src/$(INAC_LIB)
	@for i in $(DIRS); do $(MAKE) -C $$i $(INAC_BUILD_TYPE); done
	@echo === Done ===



release: CFLAGS += $(INAC_CC_RELEASE_FLAGS)
	export CFLAGS
release: INAC_BUILD_TYPE = release
	export INAC_BUILD_TYPE
release: all

debug: CFLAGS += $(INAC_CC_DEBUG_FLAGS) -DINA_TRACE_ENABLED=1 -DINA_TRACE_LEVEL=1 -DINA_LOG_LEVEL=4 -DINA_DGBMSG_ASSERT=0
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
	@find . -name "*.gnco" -type f -delete
	@find . -name "*.gcda" -type f -delete
	@find . -name "*.gcov" -type f -delete
 

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

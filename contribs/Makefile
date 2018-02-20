#
# Copyright (c) 2013-2018, INAOS GmbH
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

OS := $(shell uname -s)

all: 
	@echo building contribs...
	$(MAKE) -C luajit amalg
ifneq ($(OS), Darwin)
	$(MAKE) -C cpu-topology
endif
	$(MAKE) -C lz4
	$(MAKE) -C miniz
	$(MAKE) -C timerwheel
	$(MAKE) -C xxhash
	$(MAKE) -C falkhash
	$(MAKE) -C memhash
	$(MAKE) -C t1ha

test: all

release: all
	
debug: XCFLAGS+= -DLUA_USE_APICHECK
	export XCFLAGS
debug: LCFLAGS+= -g
	export LCFLAGS
debug: all
	
.PHONY: clean luajit 

clean:
	@echo cleaning contribs...
	$(MAKE) clean -C luajit
ifneq ($(OS), Darwin)
	$(MAKE) clean -C cpu-topology
endif
	$(MAKE) clean -C lz4
	$(MAKE) clean -C miniz
	$(MAKE) clean -C timerwheel
	$(MAKE) clean -C xxhash
	$(MAKE) clean -C falkhash
	$(MAKE) clean -C memhash


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
# Set local  settinsg
# ****************************************************************************

all: makeheaders_tool anet skiplist bstring sqlite luajit
	@echo building contribs

makeheaders_tool:
	@$(MAKE) -C makeheaders

luajit:
	@$(MAKE) -C luajit

sqlite:
	@$(MAKE) -C sqlite

bstring:
	@$(MAKE) -C bstring

anet:
	@$(MAKE) -C anet

skiplist:
	@$(MAKE) -C skiplist

sds:
	@$(MAKE) -C sds

test: all
	
.PHONY: clean makeheaders_tool bstring sqlite luajit sds anet skiplist

clean:
	@$(MAKE) clean -C makeheaders
	@$(MAKE) clean -C bstring
	@$(MAKE) clean -C skiplist
	@$(MAKE) clean -C anet
	@$(MAKE) clean -C sds
	@$(MAKE) clean -C luajit
	@$(MAKE) clean -C sqlite

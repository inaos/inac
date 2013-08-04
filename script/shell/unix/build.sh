#!/bin/sh
#
# Copyright (c) 2013, INAOS GmbH
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
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

#
# Required input
# --------------
# 1. Build-Stage (all/test/clean/dist) and Build-Type(release/debug)
#   command-line args
#
# 2. Environment variables that define what needs to be done and where
#
# Input environemnt variables
# ---------------------------
# * INAC_BUILD_NAME: Name of the artefact to be built - Required
# * INAC_BUILD_PROJECT_DIR: Directory reference for detailed artefacts
#   - Required
# * INAC_BUILD_SOURCE_DIR: Directory relative to PROJECT_DIR - Optional
# * INAC_BUILD_TEST_SOURCE_DIR: Directory relative to PROJECT_DIR - Optional
# * INAC_BUILD_TEST_SUITE_EXEC: Executable that invokes the test-suite
#                                 relative to PROJECT_DIR - Optional
# * INAC_BUILD_BUILD_TOOL: Either 'cmake-make','make' or 0make.sh  - 
#                                  or 'luajit'. optional
# * INAC_BUILD_SOURCE_DIR: Directory relative to PROJECT_DIR - Optional
# * INAC_BUILD_LIB_NAME: Name of the library where the lua byte-code is
#                             stored - Optional
# * INAC_BUILD_CODE_GEN_SCRIPT: Lua script that will be invoked before
#                               compilation, relative from PROJECT_DIR
#                               Optional
#
# Environment variable rules
# --------------------------
# * You must define either INAC_BUILD_SOURCE_DIR
# * If you define INAC_BUILD_SOURCE_DIR or INAC_BUILD_TEST_SOURCE_DIR 
#   you have to define -> INAC_BUILD_C_BUILD_TOOL
# * If you define INAC_BUILD_SOURCE_DIR then you have to define
#   -> INAC_BUILD_LIB_NAME
# * If you define INAC_BUILD_TEST_SOURCE_DIR you have to define
#  -> INAC_BUILD_TEST_SUITE_EXEC
#
# Build stages
# ------------
#
# The build stages mimic the well known values from UNIX makefiles:
#
# 'all' = Compiles and links all code include tests, also invokes 
#         code-generation if required
# 'test' = Execute the tests
# 'clean' = Delete all the build artefacts
# 'dist' = Create distribuition package (source tarball). Execute 'clean', 
# 'all' and 'test' targets
#
# General facts:
# --------------
# * The whole build script is restartable, you can invoke all as many times 
#   as you like only new stuff will be built
#
# -------------------------------------------------------------------------

# set variables according to input
export INAC_BUILD_LUAJIT=$INAC_HOME/contribs/luajit/src/luajit
export INAC_BUILD_LUAJIT_DIR=$INAC_HOME/contribs/luajit/src/jit

if [ ! -n "$ORIGINAL_LUA_PATH" ]; then
      export ORIGINAL_LUA_PATH=$LUA_PATH
fi

export LUA_PATH="$INAC_HOME/contribs/luajit/src/?.lua;$ORIGINAL_LUA_PATH"
# Eval stage
if [ ! -z "$1" ]; then
    export INAC_BUILD_STAGE="$1"
fi

if [ ! -z "$2" ]; then
    export INAC_BUILD_TYPE="$2"
fi

if [ "eval_param" != "$3" ]; then

    cd "$INAC_BUILD_PROJECT_DIR"

    echo "Running build $INAC_BUILD_NAME: $INAC_BUILD_TYPE - $INAC_BUILD_STAGE"
        
    OLD_DIR="$(pwd)"

    # Check whenever we neer to tunn a code generator
    if [ ! -z "$INAC_BUILD_CODE_GEN_SCRIPT" ]; then
        $INAC_BUILD_LUAJIT "$INAC_BUILD_PROJECT_DIR/$INAC_BUILD_CODE_GEN_SCRIPT" $INAC_BUILD_PROJECT_DIR
        if [ "$?" -ne "0" ]; then
         echo "Failed to run generator script"
         exit 1
        fi
    fi
    unset INAC_BUILD_CODE_GEN_SCRIPT
    
    # Run the build "tool" if any
    if [ -f "$(dirname $INAC_BUILD_SCRIPT)/$INAC_BUILD_TOOL.tool" ]; then
        . "$(dirname $INAC_BUILD_SCRIPT)/$INAC_BUILD_TOOL.tool"
    elif [ "$INAC_BUILD_TOOL" == "make.sh" ]; then
        . make.sh $INAC_BUILD_STAGE
    else    
        make $INAC_BUILD_STAGE
    fi

    cd "$OLD_DIR"
fi

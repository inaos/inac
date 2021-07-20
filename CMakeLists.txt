#
# Copyright 2018-2020 INAOS GmbH, Thalwil
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
cmake_minimum_required(VERSION 3.7)
project(inac VERSION 1.0.17)
include("inac.cmake")
inac_version_header(include/libinac/version.h)
inac_enable_aes()
inac_enable_sse4()

inac_enable_trace(Debug 3)
inac_enable_trace(RelWithDebInfo 1)
inac_enable_trace(Release 1)
inac_enable_log(Debug 4)
inac_enable_log(RelWithDebInfo 3)
inac_enable_log(Release 3)

inac_add_contrib_lib(miniz)
inac_add_contrib_lib(lz4)
inac_add_contrib_lib(timerwheel)
inac_add_contrib_lib(xxhash)
inac_add_contrib_lib(falkhash)
inac_add_contrib_lib(t1ha SOURCE_ROOT src)

inac_add_luafiles(inac_lua
		"${DEPS_DIR}/luajit/src/jit/bc.lua"
		"${DEPS_DIR}/luajit/src/jit/bcsave.lua"
		"${DEPS_DIR}/luajit/src/jit/dis_arm.lua"
		"${DEPS_DIR}/luajit/src/jit/dis_mips.lua"
		"${DEPS_DIR}/luajit/src/jit/dis_mipsel.lua"
		"${DEPS_DIR}/luajit/src/jit/dis_ppc.lua"
		"${DEPS_DIR}/luajit/src/jit/dis_x64.lua"
		"${DEPS_DIR}/luajit/src/jit/dis_x86.lua"
		"${DEPS_DIR}/luajit/src/jit/dump.lua"
		"${DEPS_DIR}/luajit/src/jit/v.lua"
		"${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit-external/src/jit/vmdef.lua"
		"${DEPS_DIR}/luatest/luamock.lua"
		"${DEPS_DIR}/luatest/luaspec.lua"
		"${SRC_DIR}/lconffile.lua"
		"${SRC_DIR}/ldebug.lua"
		"${SRC_DIR}/lprocqry.lua"
		"${SRC_DIR}/lsocket.lua"
		"${SRC_DIR}/ltest.lua")

inac_add_contribs_headers(
		luajit/src/lua.h
		luajit/src/luaconf.h
		luajit/src/luajit.h
		luajit/src/lauxlib.h
		luajit/src/lualib.h
		miniz/miniz.h)

file(GLOB src ${SRC_DIR}/*.c)
add_library(inac_c ${src})
set_target_properties(
		inac_c
		PROPERTIES
		COMPILE_DEFINITIONS INA_LIB=1)

inac_add_contrib_lib_ex_win32(luajit DEPENDS inac_c SOURCE_ROOT /src COMMAND src/msvcbuild.bat COMMAND_ARGS static LIBNAME lua51 BUILD_TYPES MinSizeRel Release RelWithDebInfo)
inac_add_contrib_lib_ex_win32(luajit DEPENDS inac_c SOURCE_ROOT /src COMMAND src/msvcbuild_debug.bat COMMAND_ARGS static LIBNAME lua51d BUILD_TYPES  Debug)
if (APPLE)
inac_add_contrib_lib_ex_unix(luajit DEPENDS inac_c SOURCE_ROOT /src COMMAND_ARGS MACOSX_DEPLOYMENT_TARGET=10.14 amalg)
else(APPLE)
inac_add_contrib_lib_ex_unix(luajit DEPENDS inac_c SOURCE_ROOT /src COMMAND_ARGS amalg)
endif(APPLE)
if (NOT APPLE)
inac_add_contrib_lib_ex_unix(memhash DEPENDS inac_c OMIT_PREFIX)
inac_add_contrib_lib_ex_win32(memhash DEPENDS inac_c COMMAND memhash_64.bat ARCH x86_64)
inac_add_contrib_lib_ex_win32(memhash DEPENDS inac_c COMMAND memhash_32.bat ARCH x86)
endif()
inac_add_contrib_lib_ex_linux(cpu-topology DEPENDS inac_c OMIT_PREFIX)
inac_add_contrib_lib_ex_win32(cpu-topology DEPENDS inac_c COMMAND inac_64.bat ARCH x86_64 COMMAND_ARGS /MD BUILD_TYPES  Release RelWithDebInfo MinSizeRel)
inac_add_contrib_lib_ex_win32(cpu-topology DEPENDS inac_c COMMAND inac_32.bat ARCH x86 COMMAND_ARGS /MD BUILD_TYPES  Release RelWithDebInfo MinSizeRel)
inac_add_contrib_lib_ex_win32(cpu-topology DEPENDS inac_c COMMAND inac_64.bat ARCH x86_64 COMMAND_ARGS /MDd BUILD_TYPES  Debug)
inac_add_contrib_lib_ex_win32(cpu-topology DEPENDS inac_c COMMAND inac_32.bat ARCH x86 COMMAND_ARGS /MDd BUILD_TYPES  Debug)


inac_merge_static_libs(inac inac_c ${INAC_LIBS})

inac_add_tests(inac)
inac_add_benchmarks(inac)
inac_add_tools(inac)
inac_add_examples(inac)
inac_add_luafiles(idoc_lua "${CMAKE_SOURCE_DIR}/tools/lidoc.lua")
target_link_libraries(idoc inac ${INAC_DEPENDENCY_LIBS} idoc_lua ${PLATFORM_LIBS})


inac_post_copy_file(tests hashtable.conf)
inac_post_copy_file(tests test_log.conf)
inac_post_copy_file(tests test_filepath.conf)
inac_post_copy_file(tests test_conffile_anonymous_section.conf)
inac_post_copy_file(tests test_ljit.lua)
inac_post_copy_file(tests test_lsocket.lua)
inac_post_copy_file_unix(tests tests.conf)
inac_post_copy_file_win32(tests tests.conf DEST tests.exe.conf)

install(TARGETS inac
		DESTINATION lib
		COMPONENT libraries)
		
# Also package luajit		
if(WIN32)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(LUAJIT_EXE "luajitd.exe")
	else()
		set(LUAJIT_EXE "luajit.exe")
	endif()
else()
	set(LUAJIT_EXE "luajit")
endif()		
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit-external/src/${LUAJIT_EXE}
        DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit-external/src/jit/
        DESTINATION bin/jit)


inac_package()

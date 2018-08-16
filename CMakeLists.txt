cmake_minimum_required (VERSION 3.1)
project (inac)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/inac.cmake")
	if (NOT EXISTS "${CMAKE_SOURCE_DIR}/inac.cmake")
		message(STATUS "Downloading inac.cmake from https://github.com/inaos/inac-cmake")
		file(DOWNLOAD "https://raw.githubusercontent.com/inaos/inac-cmake/0.1.0/inac.cmake"
			"${CMAKE_BINARY_DIR}/inac.cmake" STATUS DS)
		if(NOT "${DS}"  MATCHES "0;")
			file(REMOVE "${CMAKE_BINARY_DIR}/inac.cmake")
			message(FATAL_ERROR "Failed to download inac.cmake")
		endif()
	else()
		message(STATUS "Use local inac.cmake")
		configure_file("${CMAKE_SOURCE_DIR}/inac.cmake" "${CMAKE_BINARY_DIR}/inac.cmake" COPYONLY)
	endif()
endif()
include("inac.cmake")

#inac_enable_verbose()
inac_enable_aes()
inac_enable_sse4()
inac_enable_trace(Debug 1)
inac_enable_log(Debug 4)
inac_enable_log(RelWithDebInfo 3)

inac_add_contrib_lib(miniz)
inac_add_contrib_lib(lz4)
inac_add_contrib_lib(timerwheel)
inac_add_contrib_lib(xxhash)
inac_add_contrib_lib(falkhash)
inac_add_contrib_lib(t1ha SOURCE_ROOT src)


inac_platform_libs_for_win("Ws2_32.lib;Psapi.lib;Iphlpapi.lib;winmm.lib;DbgHelp.lib")
inac_platform_libs_for_linux("-lrt -ldl -lm")
inac_platform_libs_for_osx("-ldl -lm")

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
		"${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit/src/jit/vmdef.lua"
		"${DEPS_DIR}/luatest/luamock.lua"
		"${DEPS_DIR}/luatest/luaspec.lua"
		"${SRC_DIR}/lconffile.lua"
		"${SRC_DIR}/ldebug.lua"
		"${SRC_DIR}/lprocqry.lua"
		"${SRC_DIR}/lsocket.lua"
		"${SRC_DIR}/ltest.lua")

file(GLOB src ${SRC_DIR}/*.c)
add_library(inac ${src})
target_link_libraries(inac ${INAC_LIBS})
set_target_properties(
		inac
		PROPERTIES
		COMPILE_DEFINITIONS INA_LIB=1)

inac_add_contrib_lib_ex_win32(inac luajit src/ YES src/msvcbuild.bat)
inac_add_contrib_lib_ex_unix(inac luajit src/ YES make amalg)
inac_add_contrib_lib_ex_unix(inac memhash "/"  NO make)
inac_add_contrib_lib_ex_win32(inac memhash "/"  NO memhash_64.bat)
inac_add_contrib_lib_ex_linux(inac cpu-topology "/" NO make)
inac_add_contrib_lib_ex_win32(inac cpu-topology "/" NO inac_64.bat)

inac_add_tests()
inac_add_benchmarks()
inac_add_tools()

inac_post_copy_file(tests tests.conf)
inac_post_copy_file(tests test_filepath.conf)
#inac_post_copy_file(tests test.exe.conf)
inac_post_copy_file(tests test_conffile_anonymous_section.conf)
inac_post_copy_file(tests test_ljit.lua)
inac_post_copy_file(tests test_lsocket.lua)

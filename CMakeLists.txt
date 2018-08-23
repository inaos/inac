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
inac_set_version(0 9 0 OUTPUT ${CMAKE_SOURCE_DIR}/include/libinac/version.h)
inac_enable_verbose()
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
		"${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit-external/src/jit/vmdef.lua"
		"${DEPS_DIR}/luatest/luamock.lua"
		"${DEPS_DIR}/luatest/luaspec.lua"
		"${SRC_DIR}/lconffile.lua"
		"${SRC_DIR}/ldebug.lua"
		"${SRC_DIR}/lprocqry.lua"
		"${SRC_DIR}/lsocket.lua"
		"${SRC_DIR}/ltest.lua")

inac_add_contribs_headers(
		timerwheel/timeout_queue.h
		timerwheel/timeout.h
		luajit/src/lua.h
		luajit/src/luaconf.h
		luajit/src/luajit.h
		luajit/src/lauxlib.h
		luajit/src/lualib.h)

file(GLOB src ${SRC_DIR}/*.c)
add_library(inac_c ${src})
target_link_libraries(inac_c ${INAC_LIBS})
set_target_properties(
		inac_c
		PROPERTIES
		COMPILE_DEFINITIONS INA_LIB=1)

inac_add_contrib_lib_ex_win32(luajit DEPENDS inac_c SOURCE_ROOT /src COMMAND src/msvcbuild.bat COMMAND_ARGS static LIBNAME lua51 BUILD_TYPES release Release RelWithDebInfo)
inac_add_contrib_lib_ex_win32(luajit DEPENDS inac_c SOURCE_ROOT /src COMMAND src/msvcbuild_debug.bat COMMAND_ARGS static LIBNAME lua51d BUILD_TYPES debug Debug)
inac_add_contrib_lib_ex_unix(luajit DEPENDS inac_c SOURCE_ROOT /src COMMAND_ARGS amalg)
inac_add_contrib_lib_ex_unix(memhash DEPENDS inac_c OMIT_PREFIX)
inac_add_contrib_lib_ex_win32(memhash DEPENDS inac_c COMMAND memhash_64.bat ARCH x86_64)
inac_add_contrib_lib_ex_win32(memhash DEPENDS inac_c COMMAND memhash_32.bat ARCH x86)
inac_add_contrib_lib_ex_linux(cpu-topology DEPENDS inac_c OMIT_PREFIX)
inac_add_contrib_lib_ex_win32(cpu-topology DEPENDS inac_c COMMAND inac_64.bat ARCH x86_64 COMMAND_ARGS /MD BUILD_TYPES release Release RelWithDebInfo)
inac_add_contrib_lib_ex_win32(cpu-topology DEPENDS inac_c COMMAND inac_32.bat ARCH x86 COMMAND_ARGS /MD BUILD_TYPES release Release RelWithDebInfo)
inac_add_contrib_lib_ex_win32(cpu-topology DEPENDS inac_c COMMAND inac_64.bat ARCH x86_64 COMMAND_ARGS /MDd BUILD_TYPES debug Debug)
inac_add_contrib_lib_ex_win32(cpu-topology DEPENDS inac_c COMMAND inac_32.bat ARCH x86 COMMAND_ARGS /MDd BUILD_TYPES debug Debug)


inac_merge_libs(inac inac_c ${INAC_LIBS})

inac_add_tests(inac)
inac_add_benchmarks(inac)
inac_add_tools(inac)

inac_post_copy_file(tests test_filepath.conf)
inac_post_copy_file(tests test_conffile_anonymous_section.conf)
inac_post_copy_file(tests test_ljit.lua)
inac_post_copy_file(tests test_lsocket.lua)
inac_post_copy_file_unix(tests tests.conf)
inac_post_copy_file_win32(tests tests.conf DEST tests.exe.conf)

if (MSVC)
	install(TARGETS inac
			DESTINATION libs
			COMPONENT libraries)
else()
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libinac.a
    	    DESTINATION lib
        	COMPONENT libraries)
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/libinac
        DESTINATION include
		FILES_MATCHING
		PATTERN *.h)

inac_package()

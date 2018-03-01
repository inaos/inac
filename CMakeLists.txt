cmake_minimum_required (VERSION 3.1)
project (inac)
include(inac.cmake)

inac_enable_verbose()
inac_enable_aes()
inac_enable_sse4()
inac_enable_trace(Debug 4)
inac_enable_log(Debug 4)
inac_enable_log(Release 3)

inac_platform_libs_for_win(Ws2_32.lib Psapi.lib Iphlpapi.lib winmm.lib DbgHelp.lib)
inac_platform_libs_for_linux("")
inac_platform_libs_for_osx("")

inac_add_contrib_lib(miniz)
inac_add_contrib_lib(lz4)
inac_add_contrib_lib(timerwheel)
inac_add_contrib_lib(xxhash)
inac_add_contrib_lib(falkhash)
inac_add_contrib_lib(t1ha src/)
inac_add_contrib_lib_win32(cpu-topology)

file(GLOB lua_src ${SRC_DIR}/*.lua)
foreach(lua_src ${ls})
	add_custom_command (
			OUTPUT  ${ld}.o DEPENDS ${ls}
			COMMAND luajit -b ${ls} ">${ls}.o" )
endforeach()

file(GLOB src ${SRC_DIR}/*.c)
add_library(inac ${src})
target_link_libraries(inac ${INAC_LIBS})
set_target_properties(
		inac
		PROPERTIES
		COMPILE_DEFINITIONS INA_LIB=1)

inac_add_contrib_lib_ex_win32(luajit src/ YES src/msvcbuild.bat)
inac_add_contrib_lib_ex_unix(luajit src/ YES make amalg)

inac_add_contrib_lib_ex_unix(memhash "/"  NON make)
inac_add_contrib_lib_ex_linux(cpu-topology "/" NON make)

inac_add_tests()
inac_add_benchmarks()
inac_add_tools()

inac_post_copy_file(tests  test.conf)
inac_post_copy_file(tests test.exe.conf)
inac_post_copy_file(tests test_conffile_anonymous_section.conf)
inac_post_copy_file(tests test_ljit.lua)
inac_post_copy_file(tests test_lsocket.lua)

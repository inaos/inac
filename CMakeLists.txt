cmake_minimum_required (VERSION 2.6)
project (inac)
include(inac.cmake)

inac_enable_aes()
inac_enable_sse4()

inac_add_contrib_lib(miniz)
inac_add_contrib_lib(lz4)
inac_add_contrib_lib(timerwheel)
inac_add_contrib_lib(xxhash)
inac_add_contrib_lib(falkhash)
inac_add_contrib_lib(t1ha src/)
inac_add_luajit(luajit)

# Intel cpu-topology
#if(WIN32)
#set (ICT_SRC_DIR ${DEPS_DIR}/cpu-topology)
#set (ICT_SRC     ${ICT_SRC_DIR}/cpu_topo.c ${ICT_SRC_DIR}/util_os.c)
#add_library(${ICT_LIB_NAME} ${ICT_SRC})
#endif(WIN32)

file(GLOB src ${SRC_DIR}/*.c)
add_library(inac ${src})
target_link_libraries(inac ${INAC_LIBS})
set_target_properties(
		inac
		PROPERTIES
		COMPILE_DEFINITIONS INA_LIB=1)


inac_add_tests()
inac_add_benchmarks()
inac_add_tools()

inac_copy_file(tests test.conf)
inac_copy_file(tests test.exe.conf)
inac_copy_file(tests test_conffile_anonymous_section.conf)
inac_copy_file(tests test_ljit.lua)
inac_copy_file(tests test_lsocket.lua)

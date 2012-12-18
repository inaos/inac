cmake_minimum_required (VERSION 2.6)
project (inac)

set (DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src")

set (ANET_INCLUDE_DIR "${DEPS_DIR}/anet")
set (BSTRING_INCLUDE_DIR "${DEPS_DIR}/bstring")

include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/include" "${ANET_INCLUDE_DIR}" "${BSTRING_INCLUDE_DIR}")

#if(WIN32)
#	set (PLATFORM_LIBS Ws2_32.lib Psapi.lib Iphlpapi.lib)
#endif(WIN32)

#Preprocessor
add_definitions(-DINA_OS_WIN32 -DDEBUG -DTRACE_ENABLED -DINA_LIB -D_CRT_SECURE_NO_WARNINGS)

# BEGIN dependencies

# BSTRING
set (BSTRING_SRC_DIR ${DEPS_DIR}/bstring)
set (BSTRING_SRC ${BSTRING_SRC_DIR}/bsafe.c ${BSTRING_SRC_DIR}/bstraux.c ${BSTRING_SRC_DIR}/bstrlib.c)
add_library(bstring ${BSTRING_SRC})

# Makeheaders
add_library(makeheaders ${DEPS_DIR}/makeheaders/makeheaders.c)

# END dependencies

set (LIBINAC_SRC ${SRC_DIR}/error.c ${SRC_DIR}/memory.c ${SRC_DIR}/lib.c ${SRC_DIR}/ullc.c 
                 ${SRC_DIR}/string/bstring.c ${SRC_DIR}/string/cstring.c ${SRC_DIR}/util.c
				 ${SRC_DIR}/iscp.c ${SRC_DIR}/net.c)

add_library(libinac ${LIBINAC_SRC})

# Memory Test
#add_executable(test_memory ${SRC_DIR}/test_mempool.c)
#target_link_libraries(test_memory memory)




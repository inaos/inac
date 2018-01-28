cmake_minimum_required (VERSION 2.6)
project (inac)

set (DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}"
					"${DEPS_DIR}")

set(ANET_LIB_NAME "anet")
set(LZ4_LIB_NAME "lz4")
set(MINIZ_LIB_NAME "miniz")
set(ICT_LIB_NAME "cpu-topology")
set(TIMER_WHEEL_LIB_NAME "timerwheel")
set(XXHASH_LIB_NAME "xxhash")
set(FALKHASH_LIB_NAME "falkhash")
set(INAC_LIB_NAME "libinac_c")
set(INAC_TESTLIB_NAME "testinac")

#set build-type specific variables
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
	SET(CMAKE_BUILD_TYPE RelWithDebInfo)
	#Preprocessor
	add_definitions(-DINA_LIB -DINA_LOG_ENABLED -DINA_LOG_LEVEL=3)
else()
	#Preprocessor
	add_definitions(-DDEBUG -DTRACE_ENABLED -DINA_LIB -DINA_LOG_ENABLED -DINA_LOG_LEVEL=4)
endif()

if(WIN32)
	add_definitions(-DINA_OS_WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif(WIN32)

# BEGIN dependencies

# ANET
set (ANET_SRC_DIR ${DEPS_DIR}/anet)
set (ANET_SRC ${ANET_SRC_DIR}/anet.c)
add_library(${ANET_LIB_NAME} ${ANET_SRC})

# Miniz
set (MINIZ_SRC_DIR ${DEPS_DIR}/miniz)
set (MINIZ_SRC     ${MINIZ_SRC_DIR}/miniz.c)
add_library(${MINIZ_LIB_NAME} ${MINIZ_SRC})

# LZ4
set (LZ4_SRC_DIR ${DEPS_DIR}/lz4)
set (LZ4_SRC     ${LZ4_SRC_DIR}/lz4.c ${LZ4_SRC_DIR}/lz4hc.c)
add_library(${LZ4_LIB_NAME} ${LZ4_SRC})

# Intel cpu-topology
if(WIN32)
set (ICT_SRC_DIR ${DEPS_DIR}/cpu-topology)
set (ICT_SRC     ${ICT_SRC_DIR}/cpu_topo.c ${ICT_SRC_DIR}/util_os.c)
add_library(${ICT_LIB_NAME} ${ICT_SRC})
endif(WIN32)

# Timerwheel
set (TIMER_WHEEL_SRC_DIR ${DEPS_DIR}/timerwheel)
set (TIMER_WHEEL_SRC ${TIMER_WHEEL_SRC_DIR}/timeout.c)
add_library(${TIMER_WHEEL_LIB_NAME} ${TIMER_WHEEL_SRC})

# xxhash
set (XXHASH_SRC_DIR ${DEPS_DIR}/xxhash)
set (XXHASH_SRC ${XXHASH_SRC_DIR}/xxhash.c)
add_library(${XXHASH_LIB_NAME} ${XXHASH_SRC})

# falkhash
set (FALKHASH_SRC_DIR ${DEPS_DIR}/falkhash)
set (FALKHASH_SRC ${FALKHASH_SRC_DIR}/falkhash.c)
add_library(${FALKHASH_LIB_NAME} ${FALKHASH_SRC})

# END dependencies

add_definitions(-DINA_ISTRING_ENABLED -DINA_STRING_DEFINED)

# BEGIN backend configuration

if ($ENV{INAC_TIME_BACKEND} STREQUAL "time-meinberg")
	add_definitions(-DINA_MBTIME_ENABLED -DINA_TIME_DEFINED)
endif()
MESSAGE(STATUS "Time-Backend: $ENV{INAC_TIME_BACKEND}")

# END backend configuration

set (LIBINAC_SRC ${SRC_DIR}/error.c ${SRC_DIR}/memory.c ${SRC_DIR}/lib.c ${SRC_DIR}/ullc.c 
                 ${SRC_DIR}/istring.c ${SRC_DIR}/util.c ${SRC_DIR}/net.c ${SRC_DIR}/log.c
                 ${SRC_DIR}/time.c ${SRC_DIR}/time_os.c ${SRC_DIR}/timer.c ${SRC_DIR}/conffile.c 
				 ${SRC_DIR}/ljit.c ${SRC_DIR}/portable.c ${SRC_DIR}/cio.c ${SRC_DIR}/cron.c 
				 ${SRC_DIR}/ipc.c ${SRC_DIR}/cpu.c ${SRC_DIR}/compression.c 
				 ${SRC_DIR}/file.c ${SRC_DIR}/mmap.c ${SRC_DIR}/file_cursor.c ${SRC_DIR}/process.c 
				 ${SRC_DIR}/hash.c ${SRC_DIR}/dir.c ${SRC_DIR}/test.c)

add_library(${INAC_LIB_NAME} ${LIBINAC_SRC})

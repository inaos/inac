cmake_minimum_required (VERSION 2.6)
project (inac)

set (DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set (BIN_DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs-bin")
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# Includes where we do not own the source
set (MEINBERG_INCLUDE_DIR "${BIN_DEPS_DIR}/meinberg")

set (BIN_INCLUDES "${MEINBERG_INCLUDE_DIR}")

include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}"
					"${DEPS_DIR}" "${BIN_INCLUDES}")

set(BSTRING_LIB_NAME "bstring")
set(ANET_LIB_NAME "anet")
set(SKIPLIST_LIB_NAME "skiplist")
set(HTTP_LIB_NAME "http_parser")
set(SQLITE_LIB_NAME "sqlite")
set(RAPIDXML_LIB_NAME "rapidxml")
set(INAC_LIB_NAME "libinac_c")
set(INAC_TESTLIB_NAME "testinac")

#set build-type specific variables
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -DINA_LIB -D_CRT_SECURE_NO_WARNINGS -DINA_LOG_ENABLED -DINA_LOG_LEVEL=3)	
else()
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -DDEBUG -DTRACE_ENABLED -DINA_LIB -D_CRT_SECURE_NO_WARNINGS -DINA_LOG_ENABLED -DINA_LOG_LEVEL=4)
endif()

# BEGIN dependencies

# BSTRING
set (BSTRING_SRC_DIR ${DEPS_DIR}/bstring)
set (BSTRING_SRC ${BSTRING_SRC_DIR}/bsafe.c ${BSTRING_SRC_DIR}/bstraux.c ${BSTRING_SRC_DIR}/bstrlib.c)
add_library(${BSTRING_LIB_NAME} ${BSTRING_SRC})

# ANET
set (ANET_SRC_DIR ${DEPS_DIR}/anet)
set (ANET_SRC ${ANET_SRC_DIR}/anet.c)
add_library(${ANET_LIB_NAME} ${ANET_SRC})

# SKIPLIST
set (SKIPLIST_SRC_DIR ${DEPS_DIR}/skiplist)
set (SKIPLIST_SRC ${SKIPLIST_SRC_DIR}/skiplist.c)
add_library(${SKIPLIST_LIB_NAME} ${SKIPLIST_SRC})

# SQLITE
set (SQLITE_SRC_DIR ${DEPS_DIR}/sqlite)
set (SQLITE_SRC ${SQLITE_SRC_DIR}/sqlite3.c)
add_library(${SQLITE_LIB_NAME} ${SQLITE_SRC})

# HTTP-PARSER
set (HTTP_SRC_DIR ${DEPS_DIR}/http-parser)
set (HTTP_SRC ${HTTP_SRC_DIR}/http_parser.c)
add_library(${HTTP_LIB_NAME} ${HTTP_SRC})

# RAPIDXML
set (RAPIDXML_SRC_DIR ${DEPS_DIR}/rapidxml)
set (RAPIDXML_SRC ${RAPIDXML_SRC_DIR}/rapidxml.c)
add_library(${RAPIDXML_LIB_NAME} ${RAPIDXML_SRC})

# END dependencies

# BEGIN backend configuration

if ($ENV{INAC_TIME_BACKEND} STREQUAL "time-meinberg")
	add_definitions(-DINA_TIME_MEINBERG_ENABLED -DINA_TIME_DEFINED)
endif()
MESSAGE(STATUS "Time-Backend: $ENV{INAC_TIME_BACKEND}")

# END backend configuration

set (LIBINAC_SRC ${SRC_DIR}/error.c ${SRC_DIR}/memory.c ${SRC_DIR}/lib.c ${SRC_DIR}/ullc.c 
                 ${SRC_DIR}/cstring.c ${SRC_DIR}/util.c
                 ${SRC_DIR}/iscp.c ${SRC_DIR}/net.c ${SRC_DIR}/log.c ${SRC_DIR}/time.c
				 ${SRC_DIR}/time_os.c ${SRC_DIR}/time_meinberg.c
				 ${SRC_DIR}/timer.c ${SRC_DIR}/conffile.c ${SRC_DIR}/http.c ${SRC_DIR}/ljit.c 
				 ${SRC_DIR}/portable.c ${SRC_DIR}/cio.c ${SRC_DIR}/test.c  ${SRC_DIR}/xml.c 
				 ${SRC_DIR}/cron.c)

add_library(${INAC_LIB_NAME} ${LIBINAC_SRC})


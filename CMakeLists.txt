cmake_minimum_required (VERSION 2.6)
project (inac)

set (DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src")

set (ANET_INCLUDE_DIR "${DEPS_DIR}/anet")
set (BSTRING_INCLUDE_DIR "${DEPS_DIR}/bstring")
set (SKIPLIST_INCLUDE_DIR "${DEPS_DIR}/skiplist")
set (HTTP_INCULDE_DIR "${DEPS_DIR}/http-parser")
set (RAPIDXML_INCLUDE_DIR "${DEPS_DIR}/rapidxml")


include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}"
					"${DEPS_DIR}")

set(BSTRING_LIB_NAME "bstring")
set(ANET_LIB_NAME "anet")
set(SKIPLIST_LIB_NAME "skiplist")
set(HTTP_LIB_NAME "http_parser")
set(INAC_LIB_NAME "libinac_c")
set(INAC_TESTLIB_NAME "testinac")

#set build-type specific variables
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -DINA_LIB -D_CRT_SECURE_NO_WARNINGS)	
else()
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -DDEBUG -DTRACE_ENABLED -DINA_LIB -D_CRT_SECURE_NO_WARNINGS)
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

# HTTP-PARSER
set (HTTP_SRC_DIR ${DEPS_DIR}/http-parser)
set (HTTP_SRC ${HTTP_SRC_DIR}/http_parser.c)
add_library(${HTTP_LIB_NAME} ${HTTP_SRC})

# END dependencies

set (LIBINAC_SRC ${SRC_DIR}/error.c ${SRC_DIR}/memory.c ${SRC_DIR}/lib.c ${SRC_DIR}/ullc.c 
                 ${SRC_DIR}/cstring.c ${SRC_DIR}/util.c
                 ${SRC_DIR}/iscp.c ${SRC_DIR}/net.c ${SRC_DIR}/log.c ${SRC_DIR}/time.c
				 ${SRC_DIR}/timer.c ${SRC_DIR}/conffile.c ${SRC_DIR}/http.c ${SRC_DIR}/ljit.c 
				 ${SRC_DIR}/portable.c ${SRC_DIR}/cio.c ${SRC_DIR}/test.c  ${SRC_DIR}/xml.c)

add_library(${INAC_LIB_NAME} ${LIBINAC_SRC})

# Test-Cases
set (TEST_SRC_DIR "${CMAKE_SOURCE_DIR}/tests")
set (TEST_SRC ${TEST_SRC_DIR}/test_error.c ${TEST_SRC_DIR}/test_iscp.c ${TEST_SRC_DIR}/test_lib.c 
	${TEST_SRC_DIR}/test_log.c ${TEST_SRC_DIR}/test_memory.c ${TEST_SRC_DIR}/test_mempool.c 
	${TEST_SRC_DIR}/test_string.c ${TEST_SRC_DIR}/test_time.c ${TEST_SRC_DIR}/test_timer.c 
	${TEST_SRC_DIR}/test_util.c ${TEST_SRC_DIR}/test_ljit.c ${TEST_SRC_DIR}/test_conffile.c 
	${TEST_SRC_DIR}/test_http.c ${TEST_SRC_DIR}/test_xml.c
)
add_library(${INAC_TESTLIB_NAME} ${TEST_SRC})

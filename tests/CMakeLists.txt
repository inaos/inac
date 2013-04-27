cmake_minimum_required (VERSION 2.6)
project (inactest)

set (DEPS_DIR "${CMAKE_SOURCE_DIR}/../contribs")

set (ANET_INCLUDE_DIR "${DEPS_DIR}/anet")
set (BSTRING_INCLUDE_DIR "${DEPS_DIR}/bstring")
set (SKIPLIST_INCLUDE_DIR "${DEPS_DIR}/skiplist")

include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/../include" "${ANET_INCLUDE_DIR}" 
                    "${BSTRING_INCLUDE_DIR}" "${SKIPLIST_INCLUDE_DIR}" "${CMAKE_SOURCE_DIR}/.." )

#set build-type specific variables
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -DINA_LIB -D_CRT_SECURE_NO_WARNINGS)
else()
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -DDEBUG -DTRACE_ENABLED -DINA_LIB -D_CRT_SECURE_NO_WARNINGS)
endif()

set(INAC_LIB "libinac.lib")
set(INAC_TESTLIB "testinac.lib")

if(WIN32)
	set (PLATFORM_LIBS Ws2_32.lib Psapi.lib Iphlpapi.lib winmm.lib)
endif(WIN32)

set (LIB_DIR "${CMAKE_SOURCE_DIR}/../buildall")
set (LIBS ${LIB_DIR}/${INAC_LIB} ${LIB_DIR}/${INAC_TESTLIB} ${LIB_DIR}/${BSTRING_LIB} 
	${LIB_DIR}/${SKIPLIST_LIB} ${LIB_DIR}/${ANET_LIB}  ${DEPS_DIR}/luajit/src/${LUAJIT_LIB})

AUX_SOURCE_DIRECTORY(. TEST_SRC)
add_executable(test ${TEST_SRC})
target_link_libraries(test ${PLATFORM_LIBS} ${LIBS})

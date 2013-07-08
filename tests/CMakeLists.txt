cmake_minimum_required (VERSION 2.6)
SET(MSVC_INCREMENTAL_DEFAULT ON)
project (inactest)

STRING(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replacementFlags
${CMAKE_EXE_LINKER_FLAGS_DEBUG})
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/INCREMENTAL:NO ${replacementFlags}" )

STRING(REPLACE "INCREMENTAL:YES" "INCREMENTAL:NO" replacementFlags3
${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO
${replacementFlags3}" )

set (DEPS_DIR "${CMAKE_SOURCE_DIR}/../contribs")

include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/../include" 
                     "${DEPS_DIR}" "${CMAKE_SOURCE_DIR}/.." )

#set build-type specific variables
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -D_CRT_SECURE_NO_WARNINGS)
else()
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -DDEBUG -DINA_TRACE_ENABLED -D_CRT_SECURE_NO_WARNINGS)
endif()

set(INAC_LIB "libinac.lib")

if(WIN32)
	set (PLATFORM_LIBS Ws2_32.lib Psapi.lib Iphlpapi.lib winmm.lib)
endif(WIN32)

set (LIB_DIR "${CMAKE_SOURCE_DIR}/../buildall")
set (LIBS ${LIB_DIR}/${INAC_LIB})

file(GLOB TEST_SRC "test_*.c" "helper_*.c" "main.c")
add_executable(test ${TEST_SRC})
target_link_libraries(test ${PLATFORM_LIBS} ${LIBS})

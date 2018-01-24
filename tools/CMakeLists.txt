cmake_minimum_required (VERSION 2.6)
SET(MSVC_INCREMENTAL_DEFAULT ON)
project (inatools)

SET( MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:NO")
STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_DEBUG}) 
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/INCREMENTAL:NO ${replacementFlags}" )
STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags3 ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ${replacementFlags3}) 
SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO ${replacementFlags3}" )
 

set (DEPS_DIR "${CMAKE_SOURCE_DIR}/../contribs")
set (BIN_DEPS_DIR "${CMAKE_SOURCE_DIR}/../contribs-bin")

include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/../include" 
                     "${DEPS_DIR}" "${CMAKE_SOURCE_DIR}/.." )

#set build-type specific variables
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
	SET(CMAKE_BUILD_TYPE RelWithDebInfo)
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -D_CRT_SECURE_NO_WARNINGS -DINA_LOG_ENABLED -DINA_LOG_LEVEL=3)
else()
	#Preprocessor
	add_definitions(-DINA_OS_WIN32 -DDEBUG -DINA_TRACE_ENABLED -D_CRT_SECURE_NO_WARNINGS -DINA_LOG_ENABLED -DINA_LOG_LEVEL=4)
endif()

set(INAC_LIB "libinac.lib")

if(WIN32)
	set (PLATFORM_LIBS Ws2_32.lib Psapi.lib Iphlpapi.lib winmm.lib DbgHelp.lib)
endif(WIN32)

# For some tools we require OpenMP
if (${CMAKE_MAJOR_VERSION} GREATER 2)
	cmake_policy(SET CMP0012 OLD)
endif()
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set (LIB_DIR "${CMAKE_SOURCE_DIR}/../buildall")
set (LIBS ${LIB_DIR}/${INAC_LIB})

add_executable(ullc_monitor ullc_monitor.c)
target_link_libraries(ullc_monitor ${PLATFORM_LIBS} ${LIBS})

add_executable(iml iml.c)
target_link_libraries(iml ${PLATFORM_LIBS} ${LIBS})

add_executable(icpu icpu.c)
target_link_libraries(icpu ${PLATFORM_LIBS} ${LIBS})

add_executable(imem imem.c)
target_link_libraries(imem ${PLATFORM_LIBS} ${LIBS})

add_executable(imem_stream imem_stream.c)
target_link_libraries(imem_stream ${PLATFORM_LIBS} ${LIBS})

add_executable(istat istat.c)
target_link_libraries(istat ${PLATFORM_LIBS} ${LIBS})

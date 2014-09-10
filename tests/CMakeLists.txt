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

set(MEINBERG_LIBS "${BIN_DEPS_DIR}/meinberg/lib/mbgdevio.lib")

set(COMMERCIAL_LIBS ${MEINBERG_LIBS})

if(WIN32)
	set (PLATFORM_LIBS Ws2_32.lib Psapi.lib Iphlpapi.lib winmm.lib DbgHelp.lib)
endif(WIN32)

# This is required because the meinberg lib has been compiled with an old version of VS
if (CMAKE_GENERATOR STREQUAL "Visual Studio 11")
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif()

set (LIB_DIR "${CMAKE_SOURCE_DIR}/../buildall")
set (LIBS ${LIB_DIR}/${INAC_LIB} ${COMMERCIAL_LIBS})

file(GLOB TEST_SRC "test_*.c" "helper_*.c" "main.c" "test.def")
add_executable(test ${TEST_SRC})
target_link_libraries(test ${PLATFORM_LIBS} ${LIBS})

add_executable(service_mgr service_mgr.c)
target_link_libraries(service_mgr ${PLATFORM_LIBS} ${LIBS})

if(WIN32) 
   add_custom_command(TARGET test POST_BUILD          
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${PROJECT_SOURCE_DIR}/test.conf"         
        $<TARGET_FILE_DIR:test>)             
     
   add_custom_command(TARGET test POST_BUILD          
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${PROJECT_SOURCE_DIR}/test.conf"         
        $<TARGET_FILE_DIR:test>/test.exe.conf)                  

   add_custom_command(TARGET test POST_BUILD          
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${PROJECT_SOURCE_DIR}/test_filepath.conf"         
        $<TARGET_FILE_DIR:test>)                  

   add_custom_command(TARGET test POST_BUILD          
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${PROJECT_SOURCE_DIR}/test_conffile_anonymous_section.conf"         
        $<TARGET_FILE_DIR:test>)                  
   add_custom_command(TARGET test POST_BUILD          
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${PROJECT_SOURCE_DIR}/test_ljit.lua"         
        $<TARGET_FILE_DIR:test>)                  
   add_custom_command(TARGET test POST_BUILD          
     COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${PROJECT_SOURCE_DIR}/test_lsocket.lua"         
        $<TARGET_FILE_DIR:test>)
 endif()
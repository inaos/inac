include(ExternalProject)
set (DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories("${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}"
        "${DEPS_DIR}")

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
    SET(CMAKE_BUILD_TYPE RelWithDebInfo)
    message(WARNING "Build type 'Release' not supported, switched to 'RelWithDebInfo'")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "debug")
    add_definitions(-DDEBUG)
endif()

if(WIN32)
    add_definitions(-DINA_OS_WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif(WIN32)

add_definitions(-DINA_OSTIME_ENABLED -DINA_TIME_DEFINED)

function(inac_enable_verbose)
    set(CMAKE_VERBOSE_MAKEFILE ON)
    message(STATUS "Verbose output enabled")
endfunction()

function(inac_platform_libs_for_win LIBS)
    if(WIN32)
        set(INAC_LIBS_LIST ${PLATFORM_LIBS})
        list(APPEND INAC_LIBS_LIST "${LIBS}")
        set(PLATFORM_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    endif()
endfunction()


function(inac_platform_libs_for_linux LIBS)
    if(LINUX)
        set(INAC_LIBS_LIST ${PLATFORM_LIBS})
        list(APPEND INAC_LIBS_LIST "${LIBS}")
        set(PLATFORM_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    endif()
endfunction()

function(inac_platform_libs_for_osx LIBS)
    if(APPLE)
        set(INAC_LIBS_LIST ${PLATFORM_LIBS})
        list(APPEND INAC_LIBS_LIST "${LIBS}")
        set(PLATFORM_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    endif()
endfunction()


#
#
#
function (inac_enable_sse4)
    if(APPLE)
        add_definitions(-msse4)
        message(STATUS "SSE4 enabled")
    endif()
endfunction(inac_enable_sse4)

#
#
#
function (inac_enable_aes)
    if(APPLE)
        add_definitions(-maes)
        message(STATUS "AES enabled")
    endif()
endfunction(inac_enable_aes)

#
#
#
function (inac_enable_trace BUILD_TYPE LEVEL)
    if(${BUILD_TYPE} STREQUAL CMAKE_BUILD_TYPE)
        message(STATUS "Tracing enabled. Level: ${LEVEL}")
        add_definitions(-DTRACE_ENABLED -DINA_TRACE_LEVEL=${LEVEL})
    endif()
endfunction()

#
#
#
function (inac_enable_log BUILD_TYPE LEVEL)
    if(${BUILD_TYPE} STREQUAL CMAKE_BUILD_TYPE)
        message(STATUS "Logging enabled. Level: ${LEVEL}")
        add_definitions(-DINA_LOG_ENABLED -DINA_LOG_LEVEL=${LEVEL})
    endif()
endfunction()

#
#
#
function(inac_add_contrib_lib libname)
    set(INAC_LIBS_LIST ${INAC_LIBS})
    list(APPEND INAC_LIBS_LIST "${libname}")
    file(GLOB src "${CMAKE_SOURCE_DIR}/contribs/${libname}/${ARGV1}*.c")
    set(INAC_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    add_library(${libname} ${src})
    message(STATUS "Added contrib lib ${libname}")
endfunction(inac_add_contrib_lib)

function(inac_add_contrib_lib_win32 libname)
    if (WIN32)
        inac_add_contrib_lib(${libname})
    endif()
endfunction()

function(inac_add_contrib_lib_linux libname)
    if (LINUX)
        inac_add_contrib_lib(${libname})
    endif()
endfunction()

function(inac_add_contrib_lib_osx libname)
    if (APPLE)
        inac_add_contrib_lib(${libname})
    endif()
endfunction()

#
#
#
function(inac_add_contrib_lib_ex DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    ExternalProject_Add(${TARGET}
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
            CONFIGURE_COMMAND ""
            URL ${CMAKE_SOURCE_DIR}/contribs/${TARGET}
            BUILD_COMMAND "${COMMAND}" "${ARGV5}"
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND ""
            )
    if(WIN32)
        set(LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/src/${TARGET}/${DIR}")
        set(prefix "")
        set(suffix ".lib")
    else()
        set(LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/src/${TARGET}/${DIR}")
        if("${PREFIX_YES_NO}" STREQUAL "YES")
            set(prefix "lib")
        else()
            set(prefix "")
        endif()
        set(suffix ".a")
    endif()

    set(LIBRARIES "${LIB_DIR}/${prefix}${TARGET}${suffix}")
    set(INAC_LIBS ${INAC_LIBS} ${LIBRARIES} PARENT_SCOPE)
    add_dependencies(${DEPENDS} ${TARGET})
    message(STATUS "Added contrib lib ${TARGET}")
endfunction()

function(inac_add_contrib_lib_ex_win32 DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    if (WIN32)
        inac_add_contrib_lib_ex(${DEPENDS} ${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} "${ARGV5}")
    endif()
endfunction()

function(inac_add_contrib_lib_ex_linux DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    if (LINUX)
        inac_add_contrib_lib_ex(${DEPENDS} ${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} "${ARGV5}")
    endif()
endfunction()

function(inac_add_contrib_lib_ex_unix DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    if (NOT WIN32)
        inac_add_contrib_lib_ex(${DEPENDS} ${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} "${ARGV5}")
    endif()
endfunction()

function(inac_add_contrib_lib_ex_osx DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    if (APPLE)
        inac_add_contrib_lib_ex(${DEPENDS} ${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} "${ARGV5}")
    endif()
endfunction()

#
#
#
function (inac_add_tests)
    remove_definitions(-DINA_LIB)
    file(GLOB src ${CMAKE_SOURCE_DIR}/tests/test_*.c ${CMAKE_SOURCE_DIR}/tests/helper_*.c)
    list(LENGTH src src_count)
    if(${src_count} EQUAL 0)
        message(WARNING "Did no found any test in ${CMAKE_SOURCE_DIR}/tests")
        return()
    endif()
    message(STATUS "Found ${src_count} files to compile into tests")
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/tests/main.c")
        if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c")
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c
                    "int main(int argc,  char** argv) { return ina_test_run(argc, argv);}"
                    )
            list(APPEND src "${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c")
            message(STATUS "Generate main.c for tests")
        endif()
    else()
        list(APPEND src "${CMAKE_SOURCE_DIR}/tests/main.c")
        message(STATUS "Do NOT generate main.c for tests")
    endif()
    add_executable(tests ${src})
    target_link_libraries(tests inac ${INAC_LIBS})
endfunction(inac_add_tests)

#
#
#
function (inac_add_benchmarks)
    remove_definitions(-DINA_LIB)
    file(GLOB src ${CMAKE_SOURCE_DIR}/tests/bench/bench_*.c)
    list(LENGTH src src_count)
    if(${src_count} EQUAL 0)
        message(WARNING "Did no found any benchmark in ${CMAKE_SOURCE_DIR}/tests/bench")
        return()
    endif()
    message(STATUS "Found ${src_count} files to compile into bench")
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/tests/bench/main.c")
        if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c")
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c
                "int main(int argc,  char** argv) { return ina_bench_run(argc, argv);}"
                )
        endif()
    else()
        list(APPEND src "${CMAKE_SOURCE_DIR}/tests/bench/main.c")
        message(STATUS "Do NOT generate main.c for benchmarks")
    endif()
    add_executable(bench ${src})
    target_link_libraries(bench  inac ${INAC_LIBS})
endfunction(inac_add_benchmarks)

#
#
#
function(inac_add_tools)
    remove_definitions(-DINA_LIB)
    message(STATUS "Platform libs: ${PLATFORM_LIBS}")
    file(GLOB src ${CMAKE_SOURCE_DIR}/tools/*.c)
    foreach(tool_src ${src})
        string(REGEX MATCH "^(.*)\\.[^.]*$" dummy ${tool_src})
        set(tool ${CMAKE_MATCH_1})
        STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/tools/" "" tool ${tool})
        add_executable(${tool} ${tool_src})
        target_link_libraries(${tool} inac ${INAC_LIBS} ${PLATFORM_LIBS})
    endforeach()
endfunction(inac_add_tools)

#
#
#
function(inac_post_copy_file TARGET FILE)
    message(STATUS "Post copy file '${FILE} for target ${TARGET}")
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/${FILE}"
            $<TARGET_FILE_DIR:${TARGET}>)
endfunction()

#
# Add lua file to compile
#
function (inac_add_luafiles DIR)
    set(ENV{LUA_PATH}  "${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit/src/?.lua" PARENT_SCOPE)
    message(STATUS "Lua Path: $ENV{LUA_PATH}")
	message(STATUS "Searching luajit in ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit/src")
	find_program(LUAJIT_CMD luajit PATHS ${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit/src 
	NO_DEFAULT_PATH)
    set(OBJECTS ${LUA_OBJECTS})
    file(GLOB src ${DIR}/*.lua)
    foreach(ls ${src})
        message(STATUS "Added ${ls} to compile")
        get_filename_component(TN ${ls} NAME_WE)
        add_custom_command (
                OUTPUT  ${ls}.o DEPENDS ${ls} luajit
                COMMAND ${LUAJIT_CMD} -b ${ls} ${ls}.o )
        add_library(${TN}_LUA STATIC ${ls}.o)
        set_source_files_properties(
                ${TN}_LUA
                PROPERTIES
                EXTERNAL_OBJECT true
                GENERATED true
        )
        set_target_properties(
                ${TN}_LUA
                PROPERTIES
                LINKER_LANGUAGE C
        )
        list(APPEND OBJECTS ${TN}_LUA)
    endforeach()
    set(LUA_OBJECTS ${OBJECTS} PARENT_SCOPE)
endfunction()

function (inac_amalg_lib LIB LIBS)
    message(STATUS "Amalg lib ${LIB} with ${LIBS}")
    ADD_LIBRARY(merged STATIC dummy.c)

    SET_TARGET_PROPERTIES(merged PROPERTIES
            STATIC_LIBRARY_FLAGS "full\path\to\lib1.lib full\path\to\lib2.lib")
endfunction()



# Merge static libraries into a big static lib. The resulting library
# should not not have dependencies on other static libraries.
# We use it in MySQL to merge mysys,dbug,vio etc into mysqlclient

MACRO(MERGE_STATIC_LIBS TARGET OUTPUT_NAME LIBS_TO_MERGE)
# To produce a library we need at least one source file.
# It is created by ADD_CUSTOM_COMMAND below and will helps
# also help to track dependencies.
SET(SOURCE_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_depends.c)
ADD_LIBRARY(${TARGET} STATIC ${SOURCE_FILE})
SET_TARGET_PROPERTIES(${TARGET} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})

SET(OSLIBS)
FOREACH(LIB ${LIBS_TO_MERGE})
GET_TARGET_PROPERTY(LIB_LOCATION ${LIB} LOCATION)
GET_TARGET_PROPERTY(LIB_TYPE ${LIB} TYPE)
IF(NOT LIB_LOCATION)
# 3rd party library like libz.so. Make sure that everything
# that links to our library links to this one as well.
LIST(APPEND OSLIBS ${LIB})
ELSE()
# This is a target in current project
# (can be a static or shared lib)
IF(LIB_TYPE STREQUAL "STATIC_LIBRARY")
SET(STATIC_LIBS ${STATIC_LIBS} ${LIB_LOCATION})
ADD_DEPENDENCIES(${TARGET} ${LIB})
# Extract dependend OS libraries
GET_DEPENDEND_OS_LIBS(${LIB} LIB_OSLIBS)
LIST(APPEND OSLIBS ${LIB_OSLIBS})
ELSE()
# This is a shared library our static lib depends on.
LIST(APPEND OSLIBS ${LIB})
ENDIF()
ENDIF()
ENDFOREACH()
IF(OSLIBS)
LIST(REMOVE_DUPLICATES OSLIBS)
TARGET_LINK_LIBRARIES(${TARGET} ${OSLIBS})
ENDIF()

# Make the generated dummy source file depended on all static input
# libs. If input lib changes,the source file is touched
# which causes the desired effect (relink).
ADD_CUSTOM_COMMAND(
OUTPUT  ${SOURCE_FILE}
COMMAND ${CMAKE_COMMAND}  -E touch ${SOURCE_FILE}
DEPENDS ${STATIC_LIBS})

IF(MSVC)
# To merge libs, just pass them to lib.exe command line.
SET(LINKER_EXTRA_FLAGS "")
FOREACH(LIB ${STATIC_LIBS})
SET(LINKER_EXTRA_FLAGS "${LINKER_EXTRA_FLAGS} ${LIB}")
ENDFOREACH()
SET_TARGET_PROPERTIES(${TARGET} PROPERTIES STATIC_LIBRARY_FLAGS
"${LINKER_EXTRA_FLAGS}")
ELSE()
GET_TARGET_PROPERTY(TARGET_LOCATION ${TARGET} LOCATION)
IF(APPLE)
# Use OSX's libtool to merge archives (ihandles universal
# binaries properly)
ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD
COMMAND rm ${TARGET_LOCATION}
COMMAND /usr/bin/libtool -static -o ${TARGET_LOCATION}
${STATIC_LIBS}
)
ELSE()
# Generic Unix, Cygwin or MinGW. In post-build step, call
# script, that extracts objects from archives with "ar x"
# and repacks them with "ar r"
SET(TARGET ${TARGET})
CONFIGURE_FILE(
${MYSQL_CMAKE_SCRIPT_DIR}/merge_archives_unix.cmake.in
${CMAKE_CURRENT_BINARY_DIR}/merge_archives_${TARGET}.cmake
@ONLY
)
ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD
COMMAND rm ${TARGET_LOCATION}
COMMAND ${CMAKE_COMMAND} -P
${CMAKE_CURRENT_BINARY_DIR}/merge_archives_${TARGET}.cmake
)
ENDIF()
ENDIF()
ENDMACRO()

# Create libs from libs.
# Merges static libraries, creates shared libraries out of convenience libraries.
# MERGE_LIBRARIES(target [STATIC|SHARED|MODULE]
#  [linklib1 .... linklibN]
#  [EXPORTS exported_func1 .... exportedFuncN]
#  [OUTPUT_NAME output_name]
#)
MACRO(MERGE_LIBRARIES)
CMAKE_PARSE_ARGUMENTS(ARG
"EXPORTS;OUTPUT_NAME"
"STATIC;SHARED;MODULE;NOINSTALL"
${ARGN}
)
LIST(GET ARG_DEFAULT_ARGS 0 TARGET)
SET(LIBS ${ARG_DEFAULT_ARGS})
LIST(REMOVE_AT LIBS 0)
IF(ARG_STATIC)
IF (NOT ARG_OUTPUT_NAME)
SET(ARG_OUTPUT_NAME ${TARGET})
ENDIF()
MERGE_STATIC_LIBS(${TARGET} ${ARG_OUTPUT_NAME} "${LIBS}")
ELSEIF(ARG_SHARED OR ARG_MODULE)
IF(ARG_SHARED)
SET(LIBTYPE SHARED)
ELSE()
SET(LIBTYPE MODULE)
ENDIF()
# check for non-PIC libraries
IF(NOT _SKIP_PIC)
FOREACH(LIB ${LIBS})
GET_TARGET_PROPERTY(${LIB} TYPE LIBTYPE)
IF(LIBTYPE STREQUAL "STATIC_LIBRARY")
GET_TARGET_PROPERTY(LIB COMPILE_FLAGS LIB_COMPILE_FLAGS)
STRING(REPLACE "${CMAKE_SHARED_LIBRARY_C_FLAGS}"
"<PIC_FLAG>" LIB_COMPILE_FLAGS ${LIB_COMPILE_FLAG})
IF(NOT LIB_COMPILE_FLAGS MATCHES "<PIC_FLAG>")
MESSAGE(FATAL_ERROR
"Attempted to link non-PIC static library ${LIB} to shared library ${TARGET}\n"
"Please use ADD_CONVENIENCE_LIBRARY, instead of ADD_LIBRARY for ${LIB}"
)
ENDIF()
ENDIF()
ENDFOREACH()
ENDIF()
CREATE_EXPORT_FILE(SRC ${TARGET} "${ARG_EXPORTS}")
ADD_LIBRARY(${TARGET} ${LIBTYPE} ${SRC})
TARGET_LINK_LIBRARIES(${TARGET} ${LIBS})
IF(ARG_OUTPUT_NAME)
SET_TARGET_PROPERTIES(${TARGET} PROPERTIES OUTPUT_NAME "${ARG_OUTPUT_NAME}")
ENDIF()
ELSE()
MESSAGE(FATAL_ERROR "Unknown library type")
ENDIF()
IF(NOT ARG_NOINSTALL)
MYSQL_INSTALL_TARGETS(${TARGET} DESTINATION "${INSTALL_LIBDIR}")
ENDIF()
SET_TARGET_PROPERTIES(${TARGET} PROPERTIES LINK_INTERFACE_LIBRARIES "")
ENDMACRO()
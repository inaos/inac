include(ExternalProject)
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories("${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}"
        "${DEPS_DIR}")

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
    SET(CMAKE_BUILD_TYPE RelWithDebInfo)
    message(WARNING "Build type 'Release' not supported, switched to 'RelWithDebInfo'")
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "debug")
    add_definitions(-DDEBUG)
endif ()

if (WIN32)
    add_definitions(-DINA_OS_WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif (WIN32)

add_definitions(-DINA_OSTIME_ENABLED -DINA_TIME_DEFINED)

function(inac_enable_verbose)
    set(CMAKE_VERBOSE_MAKEFILE ON)
    message(STATUS "Verbose output enabled")
endfunction()

function(inac_platform_libs_for_win LIBS)
    if (WIN32)
        set(INAC_LIBS_LIST ${PLATFORM_LIBS})
        list(APPEND INAC_LIBS_LIST "${LIBS}")
        set(PLATFORM_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    endif ()
endfunction()


function(inac_platform_libs_for_linux LIBS)
    if ("${CMAKE_SYSTEM}" MATCHES "Linux")
        set(INAC_LIBS_LIST ${PLATFORM_LIBS})
        list(APPEND INAC_LIBS_LIST "${LIBS}")
        set(PLATFORM_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    endif ()
endfunction()

function(inac_platform_libs_for_unix LIBS)
    if (UNIX)
        set(INAC_LIBS_LIST ${PLATFORM_LIBS})
        list(APPEND INAC_LIBS_LIST "${LIBS}")
        set(PLATFORM_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    endif ()
endfunction()

function(inac_platform_libs_for_osx LIBS)
    if (APPLE)
        set(INAC_LIBS_LIST ${PLATFORM_LIBS})
        list(APPEND INAC_LIBS_LIST "${LIBS}")
        set(PLATFORM_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    endif ()
endfunction()


#
#
#
function(inac_enable_sse4)
    if (UNIX)
        add_definitions(-msse4)
        message(STATUS "SSE4 enabled")
    endif ()
endfunction(inac_enable_sse4)

#
#
#
function(inac_enable_aes)
    if (UNIX)
        add_definitions(-maes)
        message(STATUS "AES enabled")
    endif ()
endfunction(inac_enable_aes)

#
#
#
function(inac_enable_trace BUILD_TYPE LEVEL)
    if (${BUILD_TYPE} STREQUAL CMAKE_BUILD_TYPE)
        message(STATUS "Tracing enabled. Level: ${LEVEL}")
        add_definitions(-DTRACE_ENABLED -DINA_TRACE_LEVEL=${LEVEL})
    endif()
endfunction()

#
#
#
function(inac_enable_log BUILD_TYPE LEVEL)
    if (${BUILD_TYPE} STREQUAL CMAKE_BUILD_TYPE)
        message(STATUS "Logging enabled. Level: ${LEVEL}")
        add_definitions(-DINA_LOG_ENABLED -DINA_LOG_LEVEL=${LEVEL})
    endif ()
endfunction()

#
#
#
function(inac_add_objects OBJECTS)
    set(INAC_OBJS_LIST ${INAC_OBJECTS})
    list(APPEND INAC_OBJS_LIST ${OBJECTS})
    set(INAC_OBJECTS ${INAC_OBJS_LIST} PARENT_SCOPE)
    message(STATUS "Added objects ${OBJECTS}")
endfunction(inac_add_objects)

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

macro(inac_add_contrib_lib_win32 libname)
    if (WIN32)
        inac_add_contrib_lib(${libname})
    endif ()
endmacro()

macro(inac_add_contrib_lib_linux libname)
    if ("${CMAKE_SYSTEM}" MATCHES "Linux")
        inac_add_contrib_lib(${libname})
    endif ()
endmacro()

macro(inac_add_contrib_lib_unix libname)
    if (UNIX)
        inac_add_contrib_lib(${libname})
    endif ()
endmacro()

macro(inac_add_contrib_lib_osx libname)
    if (APPLE)
        inac_add_contrib_lib(${libname})
    endif ()
endmacro()

#
#
#
function(inac_add_contrib_lib_ex DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    set(INAC_LIBS_LIST ${INAC_LIBS})
    set(LIB_DIR)
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
        if ("${PREFIX_YES_NO}" STREQUAL "YES")
            set(prefix "lib")
        else ()
            set(prefix "")
        endif ()
        set(suffix ".a")
    endif()
    add_dependencies(${DEPENDS} ${TARGET})
    list(APPEND INAC_LIBS_LIST  "${LIB_DIR}/${prefix}${TARGET}${suffix}")
    set(INAC_LIBS "${INAC_LIBS_LIST}" PARENT_SCOPE)
    message(STATUS "Added external contrib lib ${TARGET}")
endfunction()

macro(inac_add_contrib_lib_ex_win32 DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    if (WIN32)
        inac_add_contrib_lib_ex(${DEPENDS} ${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} "${ARGV5}")
    endif ()
endmacro()

macro(inac_add_contrib_lib_ex_linux DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    if ("${CMAKE_SYSTEM}" MATCHES "Linux")
        inac_add_contrib_lib_ex(${DEPENDS} ${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} "${ARGV5}")
    endif ()
endmacro()

macro(inac_add_contrib_lib_ex_unix DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    if (UNIX)
        inac_add_contrib_lib_ex(${DEPENDS} ${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} "${ARGV5}")
    endif ()
endmacro()

macro(inac_add_contrib_lib_ex_osx DEPENDS TARGET DIR PREFIX_YES_NO COMMAND)
    if (APPLE)
        inac_add_contrib_lib_ex(${DEPENDS} ${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} "${ARGV5}")
    endif ()
endmacro()

#
#
#
function(inac_add_tests)
    remove_definitions(-DINA_LIB)
    message(STATUS "Platform libs: ${PLATFORM_LIBS}")
    file(GLOB src ${CMAKE_SOURCE_DIR}/tests/test_*.c ${CMAKE_SOURCE_DIR}/tests/helper_*.c)
    list(LENGTH src src_count)
    if (${src_count} EQUAL 0)
        message(WARNING "Did no found any test in ${CMAKE_SOURCE_DIR}/tests")
        return()
    endif ()
    message(STATUS "Found ${src_count} files to compile into tests")
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/tests/main.c")
        if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c")
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c
                    "int main(int argc,  char** argv) { return ina_test_run(argc, argv);}"
                    )
            list(APPEND src "${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c")
            message(STATUS "Generate main.c for tests")
        endif ()
    else ()
        list(APPEND src "${CMAKE_SOURCE_DIR}/tests/main.c")
        message(STATUS "Do NOT generate main.c for tests")
    endif ()
    add_executable(tests ${src})
    target_link_libraries(tests inac ${INAC_OBJECTS} ${INAC_LIBS}  ${PLATFORM_LIBS} )
endfunction(inac_add_tests)

#
#
#
function(inac_add_benchmarks)
    remove_definitions(-DINA_LIB)
    message(STATUS "Platform libs: ${PLATFORM_LIBS}")
    file(GLOB src ${CMAKE_SOURCE_DIR}/tests/bench/bench_*.c)
    list(LENGTH src src_count)
    if (${src_count} EQUAL 0)
        message(WARNING "Did no found any benchmark in ${CMAKE_SOURCE_DIR}/tests/bench")
        return()
    endif ()
    message(STATUS "Found ${src_count} files to compile into bench")
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/tests/bench/main.c")
        if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c")
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c
                    "int main(int argc,  char** argv) { return ina_bench_run(argc, argv);}"
                    )
        endif ()
    else ()
        list(APPEND src "${CMAKE_SOURCE_DIR}/tests/bench/main.c")
        message(STATUS "Do NOT generate main.c for benchmarks")
    endif ()
    add_executable(bench ${src})
    target_link_libraries(bench inac ${INAC_OBJECTS} ${INAC_LIBS} ${PLATFORM_LIBS})
endfunction(inac_add_benchmarks)

#
#
#
function(inac_add_tools)
    remove_definitions(-DINA_LIB)
    message(STATUS "Platform libs: ${PLATFORM_LIBS}")
    file(GLOB src ${CMAKE_SOURCE_DIR}/tools/*.c)
    foreach (tool_src ${src})
        string(REGEX MATCH "^(.*)\\.[^.]*$" dummy ${tool_src})
        set(tool ${CMAKE_MATCH_1})
        STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/tools/" "" tool ${tool})
        add_executable(${tool} ${tool_src})
        target_link_libraries(${tool} inac ${INAC_OBJECTS} ${INAC_LIBS} ${PLATFORM_LIBS})
    endforeach ()
endfunction(inac_add_tools)

#
#
#
function(inac_post_copy_file TARGET FILE)
    message(STATUS "Post copy file '${FILE} for target ${TARGET}")
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/${TARGET}/${FILE}"
            $<TARGET_FILE_DIR:${TARGET}>)
endfunction()

#
# Add lua file to compile
#
function(inac_add_luafiles TARGET)
    if(WIN32)
        set(LUAJIT_EXE "luajit.exe")
    else()
        set(LUAJIT_EXE "luajit")
    endif()
    set(LUA_PATH "${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit/src/")
    set(LUAJIT_CMD "${LUA_PATH}${LUAJIT_EXE}")
    message(STATUS "Lua Path: ${LUAJIT_CMD}")

    set(SOURCE_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_depends.c)
    set(OBJECTS)
    foreach (ls IN LISTS ARGN)
        get_filename_component(TN ${ls} NAME)
        file(RELATIVE_PATH DN ${CMAKE_SOURCE_DIR} ${ls} )
        add_custom_command(
                OUTPUT ${ls}.o DEPENDS ${ls} luajit
                COMMAND "${LUAJIT_CMD}" -b ${ls} "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/${TN}.o" WORKING_DIRECTORY "${LUA_PATH}")
        list(APPEND OBJECTS "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/${TN}.o")
        message(STATUS "Added ${DN}/${TN} to ${TARGET}")
    endforeach ()

    # Make the generated dummy source file depended on all static input
    # libs. If input lib changes,the source file is touched
    # which causes the desired effect (relink).
    ADD_CUSTOM_COMMAND(
            OUTPUT  ${SOURCE_FILE}
            COMMAND ${CMAKE_COMMAND} -E touch ${SOURCE_FILE}
            DEPENDS ${STATIC_LIBS})

    add_library(${TARGET} STATIC ${SOURCE_FILE} ${OBJECTS})

    set(INAC_LIBS_LIST ${INAC_LIBS})
    list(APPEND INAC_LIBS_LIST ${TARGET})
    set(INAC_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
endfunction()

function(inac_amalg_lib LIB LIBS)
    message(STATUS "Amalg lib ${LIB} with ${LIBS}")
    ADD_LIBRARY(merged STATIC dummy.c)

    SET_TARGET_PROPERTIES(merged PROPERTIES
            STATIC_LIBRARY_FLAGS "full\path\to\lib1.lib full\path\to\lib2.lib")
endfunction()


include(ExternalProject)
set (DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories("${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}"
        "${DEPS_DIR}")

#set build-type specific variables
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
    SET(CMAKE_BUILD_TYPE RelWithDebInfo)
else()
    add_definitions(-DDEBUG)
endif()

if(WIN32)
    add_definitions(-DINA_OS_WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif(WIN32)

add_definitions(-DINA_OSTIME_ENABLED -DINA_TIME_DEFINED)

macro(inac_enable_verbose)
    set(CMAKE_VERBOSE_MAKEFILE ON)
    message(STATUS "Verbose output enabled")
endmacro()

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
    endif()
endfunction(inac_enable_sse4)

#
#
#
function (inac_enable_aes)
    if(APPLE)
        add_definitions(-maes)
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
endfunction(inac_add_contrib_lib)

macro(inac_add_contrib_lib_win32 libname)
    if (WIN32)
        inac_add_contrib_lib(${libname)
    endif()
endmacro()

macro(inac_add_contrib_lib_linux libname)
    if (LINUX)
        inac_add_contrib_lib(${libname)
    endif()
endmacro()

macro(inac_add_contrib_lib_osx libname)
    if (APPLE)
        inac_add_contrib_lib(${libname)
    endif()
endmacro()

#
#
#
function(inac_add_contrib_lib_ex TARGET DIR PREFIX_YES_NO COMMAND)
    ExternalProject_Add(${TARGET}
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
            CONFIGURE_COMMAND ""
            URL ${CMAKE_SOURCE_DIR}/contribs/${TARGET}
            BUILD_COMMAND "${COMMAND}" "${ARGV4}"
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
    add_dependencies(inac ${TARGET})
endfunction()

macro(inac_add_contrib_lib_ex_win32 TARGET DIR PREFIX_YES_NO COMMAND)
    if (WIN32)
        inac_add_contrib_lib_ex(${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} "${ARGV4}")
    endif()
endmacro()

macro(inac_add_contrib_lib_ex_linux TARGET DIR PREFIX_YES_NO COMMAND)
    if (LINUX)
        inac_add_contrib_lib_ex(${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} ${ARGV4})
    endif()
endmacro()

macro(inac_add_contrib_lib_ex_unix TARGET DIR PREFIX_YES_NO COMMAND)
    if (NOT WIN32)
        inac_add_contrib_lib_ex(${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} ${ARGV4})
    endif()
endmacro()

macro(inac_add_contrib_lib_ex_osx TARGET DIR PREFIX_YES_NO COMMAND)
    if (APPLE)
        inac_add_contrib_lib_ex(${TARGET} ${DIR} ${PREFIX_YES_NO} ${COMMAND} ${ARGV4})
    endif()
endmacro()

#
#
#
function (inac_add_tests)
    remove_definitions(-DINA_LIB)
    file(GLOB src ${CMAKE_SOURCE_DIR}/tests/test_*.c ${CMAKE_SOURCE_DIR}/tests/helper_*.c)
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
    file(GLOB src ${CMAKE_SOURCE_DIR}/tools/*.c)
    foreach(tool_src ${src})
        string(REGEX MATCH "^(.*)\\.[^.]*$" dummy ${tool_src})
        set(tool ${CMAKE_MATCH_1})
        STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/tools/" "" tool ${tool})
        add_executable(${tool} ${tool_src})
        target_link_libraries(${tool} inac ${INAC_LIBS})
    endforeach()
endfunction(inac_add_tools)

#
#
#
function(inac_post_copy_file TARGET FILE)
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/${FILE}"
            $<TARGET_FILE_DIR:${TARGET}>)
endfunction()

#
#
#
function (inac_add_luafiles DIR)
    set(OBJECTS ${LUA_OBJECTS})
    file(GLOB src ${DIR}/*.lua)
    foreach(ls ${src})
        message(STATUS "${ls}")
        get_filename_component(TN ${ls} NAME_WE)
        add_custom_command (
                OUTPUT  ${ls}.o DEPENDS ${ls}
                COMMAND luajit -b ${ls} ${ls}.o )
        add_library(${TN} STATIC ${ls}.o)
        SET_SOURCE_FILES_PROPERTIES(
                ${TN}_LUA
                PROPERTIES
                EXTERNAL_OBJECT true
                GENERATED true
        )
        SET_TARGET_PROPERTIES(
                ${TN}
                PROPERTIES
                LINKER_LANGUAGE C
        )
        list(APPEND OBJECTS ${TN})
    endforeach()
    set(LUA_OBJECTS ${OBJECTS} PARENT_SCOPE)
endfunction()
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

#
#
#
function(inac_add_contrib_lib_ex libname, command)
    set(INAC_LIBS_LIST ${INAC_LIBS})
    list(APPEND INAC_LIBS_LIST ${libname})
    file(GLOB src ${CMAKE_SOURCE_DIR}/contribs/${libname}/*.c)
    set(INAC_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    add_library(${libname} ${src})
endfunction(inac_add_contrib_lib_ex)

#
#
#
function (inac_add_tests)
    remove_definitions(-DINA_LIB)
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c")
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c
                "int main(int argc,  char** argv) { return ina_test_run(argc, argv);}"
                )
    endif()
    file(GLOB src ${CMAKE_SOURCE_DIR}/tests/test_.c helper_*.c)
    add_executable(tests ${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c ${src})
    target_link_libraries(tests inac)
endfunction(inac_add_tests)

#
#
#
function (inac_add_benchmarks)
    remove_definitions(-DINA_LIB)
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c")
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c
                "int main(int argc,  char** argv) { return ina_bench_run(argc, argv);}"
                )
    endif()
    file(GLOB src ${CMAKE_SOURCE_DIR}/tests/bench/bench_*.c)
    add_executable(bench ${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c ${src})
    target_link_libraries(bench inac)
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
        target_link_libraries(${tool} ${INAC_LIBS})
    endforeach()
endfunction(inac_add_tools)

#
#
#
function(inac_copy_file TARGET FILE)
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/${FILE}"
            $<TARGET_FILE_DIR:${TARGET}>)
endfunction()

#
#
#
function(inac_add_luajit)
    ExternalProject_Add(luajit_local
            PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/build/luajit
            CONFIGURE_COMMAND ""
            URL ${CMAKE_SOURCE_DIR}/contribs/luajit
            BUILD_COMMAND make
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND ""
            )
    if(WIN32)
        set(LUAJIT_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit_local/src")
        set(prefix "")
        set(suffix ".lib")
    else()
        set(LUAJIT_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit_local/src")
        set(prefix "lib")
        set(suffix ".a")
    endif()

    set(LUAJIT_LIBRARIES "${LUAJIT_LIB_DIR}/${prefix}luajit${suffix}")
    set(INAC_LIBS ${INAC_LIBS} ${LUAJIT_LIBRARIES} PARENT_SCOPE)
endfunction()
include(ExternalProject)
set (DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}"
        "${DEPS_DIR}")

#set build-type specific variables
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "release")
    SET(CMAKE_BUILD_TYPE RelWithDebInfo)
    #Preprocessor
    add_definitions(-DINA_LOG_ENABLED -DINA_LOG_LEVEL=3)
else()
    #Preprocessor
    add_definitions(-DDEBUG -DTRACE_ENABLED -DINA_LOG_ENABLED -DINA_LOG_LEVEL=4)
endif()

if(WIN32)
    add_definitions(-DINA_OS_WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATED)
endif(WIN32)

add_definitions(-DINA_OSTIME_ENABLED -DINA_TIME_DEFINED)

function (inac_enable_sse4)
    if(APPLE)
        add_definitions(-msse4)
    endif()
endfunction(inac_enable_sse4)

function (inac_enable_aes)
    if(APPLE)
        add_definitions(-maes)
    endif()
endfunction(inac_enable_aes)

function(inac_add_contrib_lib libname)
    set(INAC_LIBS_LIST ${INAC_LIBS})
    list(APPEND INAC_LIBS_LIST "${libname}")
    file(GLOB src "${CMAKE_SOURCE_DIR}/contribs/${libname}/${ARGV1}*.c")
    set(INAC_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    add_library(${libname} ${src})
endfunction(inac_add_contrib_lib)

function(inac_add_contrib_lib_ex libname, command)
    set(INAC_LIBS_LIST ${INAC_LIBS})
    list(APPEND INAC_LIBS_LIST ${libname})
    file(GLOB src ${CMAKE_SOURCE_DIR}/contribs/${libname}/*.c)
    set(INAC_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    add_library(${libname} ${src})
endfunction(inac_add_contrib_lib_ex)

function (inac_add_tests)
    remove_definitions(-DINA_LIB)
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/tests/main.c")
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tests/main.c
                "int main(int argc,  char** argv) { return ina_test_run(argc, argv);}"
                )
    endif()
    file(GLOB src ${CMAKE_SOURCE_DIR}/tests/test_.c helper_*.c)
    add_executable(tests ${CMAKE_CURRENT_BINARY_DIR}/tests/main.c ${src})
    target_link_libraries(tests inac)
endfunction(inac_add_tests)

function (inac_add_benchmarks)
    remove_definitions(-DINA_LIB)
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/bench/main.c")
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bench/main.c
                "int main(int argc,  char** argv) { return ina_bench_run(argc, argv);}"
                )
    endif()
    file(GLOB src ${CMAKE_SOURCE_DIR}/tests/bench/bench_*.c)
    add_executable(bench ${CMAKE_CURRENT_BINARY_DIR}/bench/main.c ${src})
    target_link_libraries(bench inac)
endfunction(inac_add_benchmarks)

function(inac_add_tools)
    remove_definitions(-DINA_LIB)
    file(GLOB src ${CMAKE_SOURCE_DIR}/tools/*.c)
    foreach(tool_src ${src})
        string(REGEX MATCH "^(.*)\\.[^.]*$" dummy ${tool_src})
        set(tool ${CMAKE_MATCH_1})
        STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/tools/" "" tool ${tool})
        add_executable(${tool} ${tool_src})
        target_link_libraries(${tool} inac)
    endforeach()
endfunction(inac_add_tools)

function(inac_copy_file TARGET FILE)
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/${FILE}"
            $<TARGET_FILE_DIR:${TARGET}>)
endfunction()

function(inac_add_luajit)
    ExternalProject_Add(luajitlib
            PREFIX libluajit-502
            CONFIGURE_COMMAND ""
            URL ${CMAKE_SOURCE_DIR}/contribs/luajit
            BUILD_COMMAND make
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND ""
            )
    add_library(luajit STATIC IMPORTED)
    set(INAC_LIBS_LIST ${INAC_LIBS})
    list(APPEND INAC_LIBS_LIST luajit)
    set(INAC_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    link_directories(${CMAKE_SOURCE_DIR}/build/libluajit-502/src)
endfunction()
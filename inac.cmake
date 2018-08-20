include(ExternalProject)
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INAC_CMAKE_VERSION "0.1.0")

if (MSVC)
    if (POLICY CMP0026)
        cmake_policy(SET CMP0026 OLD)
    endif()
endif()

if (MSVC)
    SET(MSVC_INCREMENTAL_DEFAULT ON)
    SET( MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:NO")
    STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/INCREMENTAL:NO ${replacementFlags}" )
    STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags3 ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
    SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ${replacementFlags3})
    SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO ${replacementFlags3}" )
endif()

message(STATUS "INAC CMake version ${INAC_CMAKE_VERSION}")

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
    set(CMAKE_VERBOSE_MAKEFILE ON PARENT_SCOPE)
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
function(inac_set_version major minor micro)
    cmake_parse_arguments(PARSE_ARGV 3 VER "" "OUTPUT" "")
    set(INAC_PROJECT_MAJOR_VERSION ${major})
    set(INAC_PROJECT_MINOR_VERSION ${minor})
    set(INAC_PROJECT_MICRO_VERSION ${micro})
    if (NOT VER_OUTPUT)
        set(VER_OUTPUT version.h)
    endif()
    set(INAC_PROJECT_MAJOR_VERSION ${major} PARENT_SCOPE)
    set(INAC_PROJECT_MINOR_VERSION ${minor} PARENT_SCOPE)
    set(INAC_PROJECT_MICRO_VERSION ${micro} PARENT_SCOPE)
    configure_file(${VER_OUTPUT}.in ${VER_OUTPUT})
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
function(inac_add_contrib_lib LIB)
    cmake_parse_arguments(PARSE_ARGV 1 LIB "" "SOURCE_ROOT" "")
    if (LIB_SOURCE_ROOT)
        set(ROOT "${LIB_SOURCE_ROOT}/")
    endif()
    set(INAC_LIBS_LIST ${INAC_LIBS})
    list(APPEND INAC_LIBS_LIST "${LIB}")
    file(GLOB src "${CMAKE_SOURCE_DIR}/contribs/${LIB}/${ROOT}*.c")
    set(INAC_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
    add_library(${LIB} ${src})
    message(STATUS "Added contrib lib ${LIB}")
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
function(inac_add_contrib_lib_ex TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 LIB OMIT_PREFIX "DEPENDS;SOURCE_ROOT;COMMAND;COMMAND_ARGS;LIBNAME;ARCH;URL" "BUILD_TYPES")

    if(LIB_ARCH)
        inac_check_arch(${LIB_ARCH})
        if (NOT (LIB_ARCH STREQUAL ${INAC_TARGET_ARCH}))
            return()
        endif()
    endif()

    if(LIB_BUILD_TYPES)
        list(FIND LIB_BUILD_TYPES "${CMAKE_BUILD_TYPE}" index)
        if (${index}  EQUAL -1)
            return()
        endif()
    endif()

    set(INAC_LIBS_LIST ${INAC_LIBS})
    set(LIB_DIR)

    if (NOT LIB_COMMAND)
        set(LIB_COMMAND make)
    endif()
    if (NOT LIB_URL)
        set(LIB_URL ${CMAKE_SOURCE_DIR}/contribs/${TARGET})
    endif()

    ExternalProject_Add(${TARGET}-external
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
            CONFIGURE_COMMAND ""
            URL ${LIB_URL}
            BUILD_COMMAND "${LIB_COMMAND}" "${LIB_COMMAND_ARGS}"
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND ""
            )

    if (NOT LIB_LIBNAME)
        set(LIBNAME ${TARGET})
    else()
        set(LIBNAME ${LIB_LIBNAME})
    endif()
    set(LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/src/${TARGET}-external/${LIB_SOURCE_ROOT}")

    if(WIN32)
        set(prefix "")
        set(suffix ".lib")
    else()
        if (${LIB_OMIT_PREFIX})
            set(prefix "")
        else ()
            set(prefix "lib")
        endif ()
        set(suffix ".a")
    endif()
    add_library(${TARGET} STATIC IMPORTED GLOBAL)
    set_target_properties(${TARGET}
            PROPERTIES
            IMPORTED_LOCATION "${LIB_DIR}/${prefix}${LIBNAME}${suffix}"
            )
    add_dependencies(${TARGET} ${TARGET}-external)
    add_dependencies(${LIB_DEPENDS} ${TARGET})
    list(APPEND INAC_LIBS_LIST  ${TARGET})
    set(INAC_LIBS "${INAC_LIBS_LIST}" PARENT_SCOPE)
    message(STATUS "Added external contrib lib ${TARGET} ${LIB_COMMAND} ${LIB_COMMAND_ARGS}")
endfunction()

macro(inac_add_contrib_lib_ex_win32 TARGET)
    if (WIN32)
        inac_add_contrib_lib_ex(${TARGET} ${ARGN})
    endif ()
endmacro()

macro(inac_add_contrib_lib_ex_linux TARGET)
    if ("${CMAKE_SYSTEM}" MATCHES "Linux")
        inac_add_contrib_lib_ex(${TARGET} ${ARGN})
    endif ()
endmacro()

macro(inac_add_contrib_lib_ex_unix TARGET)
    if (UNIX)
        inac_add_contrib_lib_ex(${TARGET} ${ARGN})
    endif ()
endmacro()

macro(inac_add_contrib_lib_ex_osx TARGET)
    if (APPLE)
        inac_add_contrib_lib_ex(${TARGET} ${ARGN})
    endif ()
endmacro()

#
#
#
function(inac_add_tests)
    if(WIN32)
        set(CMD ".\\tests.exe")
    else()
        set(CMD "./tests")
    endif()
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
    target_link_libraries(tests inac ${PLATFORM_LIBS})

    add_custom_target(runtests DEPENDS tests COMMAND "${CMD}" "--format=junit>junit.xml"  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    set_target_properties(runtests PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endfunction(inac_add_tests)

#
#
#
function(inac_add_benchmarks)
    if(WIN32)
        set(CMD ".\\bench.exe")
    else()
        set(CMD "./bench")
    endif()
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
    target_link_libraries(bench inac ${PLATFORM_LIBS})
    add_custom_target(runbenchmarks DEPENDS bench COMMAND "${CMD}" "--r=."  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    set_target_properties(runbenchmarks PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
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
        target_link_libraries(${tool} inac ${PLATFORM_LIBS})
    endforeach ()
endfunction(inac_add_tools)

#
#
#
function(inac_post_copy_file TARGET FILE)
    cmake_parse_arguments(PARSE_ARGV 2 CPY "" "DEST" "")
    if (NOT CPY_DEST)
        set(CPY_DEST ${FILE})
    endif()

    message(STATUS "Post copy file '${FILE} for target ${TARGET}")
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/${TARGET}/${CPY_DEST}"
            $<TARGET_FILE_DIR:${TARGET}>)
endfunction()

macro(inac_post_copy_file_win32 TARGET FILE)
    if (WIN32)
        inac_post_copy_file(${TARGET} ${FILE})
    endif()
endmacro()

macro(inac_post_copy_file_unix TARGET FILE)
    if (UNIX)
        inac_post_copy_file(${TARGET} ${FILE})
    endif()
endmacro()

macro(inac_post_copy_file_osx TARGET FILE)
    if (APPLE)
        inac_post_copy_file(${TARGET} ${FILE})
    endif()
endmacro()

macro(inac_post_copy_file_linux TARGET FILE)
    if ("${CMAKE_SYSTEM}" MATCHES "Linux")
        inac_post_copy_file(${TARGET} ${FILE})
    endif()
endmacro()

#
#
#
function(inac_merge_headers OUT_FILE)
    file(WRITE ${OUT_FILE}.in "")
    foreach(file ${ARGN})
        file(READ ${file} CONTENT)
        file(APPEND ${OUT_FILE}.in "${CONTENT}")
        message(STATUS "Added ${file} for merge in ${OUT_FILE}")
    endforeach()
    configure_file(${OUT_FILE}.in ${OUT_FILE} COPYONLY)
endfunction()

#
#
#
function(inac_add_contribs_headers)
    set(INAC_CONTRIBS_HEADERS "")
    foreach(file ${ARGN})
        message(STATUS "Include contrib header ${file}")
        string(CONCAT INAC_CONTRIBS_HEADERS ${INAC_CONTRIBS_HEADERS} "#include <libinac/contribs/" ${file} ">\n")
        configure_file(${DEPS_DIR}/${file} ${CMAKE_SOURCE_DIR}/include/libinac/contribs/${file} COPYONLY)
    endforeach()
    configure_file(${CMAKE_SOURCE_DIR}/include/libinac/contribs.h.in ${CMAKE_SOURCE_DIR}/include/libinac/contribs.h)
endfunction()
#
# Add lua file to compile
#
function(inac_add_luafiles TARGET)
    if(WIN32)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "debug")
            set(LUAJIT_EXE "luajitd.exe")
        else()
            set(LUAJIT_EXE "luajit.exe")
        endif()
    else()
        set(LUAJIT_EXE "luajit")
    endif()
    set(LUA_PATH "${CMAKE_CURRENT_BINARY_DIR}/luajit/src/luajit-external/src/")
    set(LUAJIT_CMD "${LUA_PATH}${LUAJIT_EXE}")
    message(STATUS "Lua Path: ${LUAJIT_CMD}")

    set(SOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_depends.c")
    set(OBJECTS)
    foreach (ls IN LISTS ARGN)
        get_filename_component(TN ${ls} NAME)
        file(RELATIVE_PATH DN ${CMAKE_SOURCE_DIR} ${ls} )
        SET_SOURCE_FILES_PROPERTIES(
                "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/${TN}.o"
                PROPERTIES
                EXTERNAL_OBJECT true
                GENERATED true
        )
        add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET}.dir/${TN}.o" DEPENDS ${ls} luajit luajit-external
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


    add_library(${TARGET} STATIC EXCLUDE_FROM_ALL ${SOURCE_FILE}  ${OBJECTS})
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES LINKER_LANGUAGE C)

    set(INAC_LIBS_LIST ${INAC_LIBS})
    list(APPEND INAC_LIBS_LIST ${TARGET})
    set(INAC_LIBS ${INAC_LIBS_LIST} PARENT_SCOPE)
endfunction()

function(inac_merge_libs LIB)
    set(SOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/${LIB}_merged.c")
    if (MSVC)
        add_library(${LIB} STATIC ${SOURCE_FILE})
        add_custom_command(
                OUTPUT  ${SOURCE_FILE}
                COMMAND ${CMAKE_COMMAND} -E touch ${SOURCE_FILE}
                DEPENDS ${ARGN})
        set(LINKER_EXTRA_FLAGS "")
        foreach(l ${ARGN})
            get_property(LIB_LOCATION TARGET ${l} PROPERTY LOCATION)
            message(STATUS "Merge lib ${l}: ${LIB_LOCATION}")
            set(LINKER_EXTRA_FLAGS "${LINKER_EXTRA_FLAGS} \"${LIB_LOCATION}\"")
         endforeach()
        set_target_properties(${LIB} PROPERTIES STATIC_LIBRARY_FLAGS "${LINKER_EXTRA_FLAGS}")
    else()
        set(C_LIB ${CMAKE_BINARY_DIR}/lib${LIB}.a)
        set(extracts "")
        foreach(l ${ARGN})
            message(STATUS "Merge lib ${l}")
            add_custom_target(${l}_extract
                    COMMAND ar -x $<TARGET_FILE:${l}>
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    DEPENDS ${l}
                    )
            list(APPEND extracts ${l}_extract)
        endforeach()
        add_custom_command(
                OUTPUT  ${SOURCE_FILE}
                COMMAND ${CMAKE_COMMAND} -E touch ${SOURCE_FILE}
                DEPENDS ${ARGN} ${extracts} )

        add_custom_target(${LIB}_merged
                COMMAND ar -qcs ${C_LIB} *.o
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS ${extracts} ${ARGN})
        add_library(${LIB} STATIC IMPORTED GLOBAL)
        add_dependencies(${LIB} ${LIB}_merged)
        set_target_properties(${LIB}
                PROPERTIES
                IMPORTED_LOCATION ${C_LIB}
                )
    endif()
endfunction()

macro(inac_check_arch arch)
    set(ARCHS "armv7;armv6;armv5;arm;i386;x86_64;ia64;ppc64;ppc;ppc64")
    list(FIND ARCHS "${arch}" index)
    if (${index} EQUAL -1)
        message(FATAL_ERROR "Invalid architectur ${arch}")
    endif()
endmacro()

# Based on the Qt 5 processor detection code, so should be very accurate
# https://qt.gitorious.org/qt/qtbase/blobs/master/src/corelib/global/qprocessordetection.h
# Currently handles arm (v5, v6, v7), x86 (32/64), ia64, and ppc (32/64)

# Regarding POWER/PowerPC, just as is noted in the Qt source,
# "There are many more known variants/revisions that we do not handle/detect."

set(INAC_ARCH_DETECT_C_CODE "
#if defined(__arm__) || defined(__TARGET_ARCH_ARM)
    #if defined(__ARM_ARCH_7__) \\
        || defined(__ARM_ARCH_7A__) \\
        || defined(__ARM_ARCH_7R__) \\
        || defined(__ARM_ARCH_7M__) \\
        || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 7)
        #error cmake_ARCH armv7
    #elif defined(__ARM_ARCH_6__) \\
        || defined(__ARM_ARCH_6J__) \\
        || defined(__ARM_ARCH_6T2__) \\
        || defined(__ARM_ARCH_6Z__) \\
        || defined(__ARM_ARCH_6K__) \\
        || defined(__ARM_ARCH_6ZK__) \\
        || defined(__ARM_ARCH_6M__) \\
        || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 6)
        #error cmake_ARCH armv6
    #elif defined(__ARM_ARCH_5TEJ__) \\
        || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 5)
        #error cmake_ARCH armv5
    #else
        #error cmake_ARCH arm
    #endif
#elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
    #error cmake_ARCH i386
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
    #error cmake_ARCH x86_64
#elif defined(__ia64) || defined(__ia64__) || defined(_M_IA64)
    #error cmake_ARCH ia64
#elif defined(__ppc__) || defined(__ppc) || defined(__powerpc__) \\
      || defined(_ARCH_COM) || defined(_ARCH_PWR) || defined(_ARCH_PPC)  \\
      || defined(_M_MPPC) || defined(_M_PPC)
    #if defined(__ppc64__) || defined(__powerpc64__) || defined(__64BIT__)
        #error cmake_ARCH ppc64
    #else
        #error cmake_ARCH ppc
    #endif
#endif
#error cmake_ARCH unknown
")

function(inac_set_target_arch arch)
    SET(INAC_TARGET_ARCH ${arch} PARENT_SCOPE)
    message(STATUS "Target architecture: ${arch}")
endfunction()

function(inac_detect_host_arch)
    if(APPLE AND CMAKE_OSX_ARCHITECTURES)
        # On OS X we use CMAKE_OSX_ARCHITECTURES *if* it was set
        # First let's normalize the order of the values

        # Note that it's not possible to compile PowerPC applications if you are using
        # the OS X SDK version 10.6 or later - you'll need 10.4/10.5 for that, so we
        # disable it by default
        # See this page for more information:
        # http://stackoverflow.com/questions/5333490/how-can-we-restore-ppc-ppc64-as-well-as-full-10-4-10-5-sdk-support-to-xcode-4

        # Architecture defaults to i386 or ppc on OS X 10.5 and earlier, depending on the CPU type detected at runtime.
        # On OS X 10.6+ the default is x86_64 if the CPU supports it, i386 otherwise.

        foreach(osx_arch ${CMAKE_OSX_ARCHITECTURES})
            if("${osx_arch}" STREQUAL "ppc" AND ppc_support)
                set(osx_arch_ppc TRUE)
            elseif("${osx_arch}" STREQUAL "i386")
                set(osx_arch_i386 TRUE)
            elseif("${osx_arch}" STREQUAL "x86_64")
                set(osx_arch_x86_64 TRUE)
            elseif("${osx_arch}" STREQUAL "ppc64" AND ppc_support)
                set(osx_arch_ppc64 TRUE)
            else()
                message(FATAL_ERROR "Invalid OS X arch name: ${osx_arch}")
            endif()
        endforeach()

        # Now add all the architectures in our normalized order
        if(osx_arch_ppc)
            list(APPEND ARCH ppc)
        endif()

        if(osx_arch_i386)
            list(APPEND ARCH i386)
        endif()

        if(osx_arch_x86_64)
            list(APPEND ARCH x86_64)
        endif()

        if(osx_arch_ppc64)
            list(APPEND ARCH ppc64)
        endif()
    else()
        file(WRITE "${CMAKE_BINARY_DIR}/arch.c" "${INAC_ARCH_DETECT_C_CODE}")

        enable_language(C)

        # Detect the architecture in a rather creative way...
        # This compiles a small C program which is a series of ifdefs that selects a
        # particular #error preprocessor directive whose message string contains the
        # target architecture. The program will always fail to compile (both because
        # file is not a valid C program, and obviously because of the presence of the
        # #error preprocessor directives... but by exploiting the preprocessor in this
        # way, we can detect the correct target architecture even when cross-compiling,
        # since the program itself never needs to be run (only the compiler/preprocessor)
        try_run(
                run_result_unused
                compile_result_unused
                "${CMAKE_BINARY_DIR}"
                "${CMAKE_BINARY_DIR}/arch.c"
                COMPILE_OUTPUT_VARIABLE ARCH
                CMAKE_FLAGS CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
        )

        # Parse the architecture name from the compiler output
        string(REGEX MATCH "cmake_ARCH ([a-zA-Z0-9_]+)" ARCH "${ARCH}")

        # Get rid of the value marker leaving just the architecture name
        string(REPLACE "cmake_ARCH " "" ARCH "${ARCH}")

        # If we are compiling with an unknown architecture this variable should
        # already be set to "unknown" but in the case that it's empty (i.e. due
        # to a typo in the code), then set it to unknown
        if (NOT ARCH)
            set(ARCH unknown)
        endif()
    endif()
    message(STATUS "Detected host architecture: ${ARCH}")
    set(INAC_HOST_ARCH "${ARCH}" PARENT_SCOPE)
endfunction()

function (inac_make_package)
    cmake_parse_arguments("P" "" "NAME;GENERATOR;VENDOR;SUMMARY;INSTALL_DIRECTORY" "")
    if (P_GENERATOR)
        set(CPACK_GENERATOR ${P_GENERATOR})
    else()
        set(CPACK_GENERATOR ZIP)
    endif()
    set(CPACK_PACKAGE_NAME ${P_NAME})
    if(P_VENDOR)
        set(CPACK_PACKAGE_VENDOR ${P_VENDOR})
    endif()
    if (P_SUMMARY)
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${P_SUMMARY})
    endif()
    set(CPACK_PACKAGE_VERSION ${INAC_PROJECT_MAJOR_VERSION}.${INAC_PROJECT_MINOR_VERSION}.${INAC_PROJECT_MICRO_VERSION})
    set(CPACK_PACKAGE_VERSION_MAJOR ${INAC_PROJECT_MAJOR_VERSION})
    set(CPACK_PACKAGE_VERSION_MINOR ${INAC_PROJECT_MINOR_VERSION})
    set(CPACK_PACKAGE_VERSION_MICRO ${INAC_PROJECT_MICRO_VERSION})
    set(CPACK_PACKAGE_INSTALL_DIRECTRORY "${P_INSTALL_DIRECTORY}}")
    include(CPack)
endfunction()
    inac_detect_host_arch()
if (NOT INAC_TARGET_ARCH)
    inac_set_target_arch(${INAC_HOST_ARCH})
endif()


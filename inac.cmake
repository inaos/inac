include(ExternalProject)

set(INAC_CMAKE_VERSION_MAJOR 0)
set(INAC_CMAKE_VERSION_MINOR 1)
set(INAC_CMAKE_VERSION_PATCH 0)

set(DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INAC_CMAKE_VERSION "${INAC_CMAKE_VERSION_MAJOR}.${INAC_CMAKE_VERSION_MINOR}.${INAC_CMAKE_VERSION_PATCH}")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "INAC CMake version ${INAC_CMAKE_VERSION}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")

if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug|Release|RelWithDebInfo")
    message(STATUS "Unsupported build type ${CMAKE_BUILD_TYPE} , allowed Debug|Release|RelWithDebInfo")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_definitions(-DINA_DEBUG=1)
endif()


if (WIN32)
    set(INAC_USER_HOME "$ENV{USERPROFILE}")
else()
    set(INAC_USER_HOME "$ENV{HOME}")
endif()


set(INAC_REPOSITORY_PATH "${INAC_USER_HOME}/.inaos/cmake")
message(STATUS "CMake package repository cache: ${INAC_REPOSITORY_PATH}")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (POLICY CMP0026)
    cmake_policy(SET CMP0026 OLD)
endif()

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -Wextra")
endif()
if ( CMAKE_C_COMPILER_ID STREQUAL "AppleClang" )
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -Wextra")
endif()

if (MSVC)
    string(REGEX REPLACE " /W[0-4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
	
	set(MSVC_INCREMENTAL_DEFAULT ON)
    set(MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:NO")

    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/INCREMENTAL:NO ${replacementFlags}" )

    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags3 ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ${replacementFlags3})
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO ${replacementFlags3}" )

    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags3 ${CMAKE_EXE_LINKER_FLAGS_RELEASE})
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${replacementFlags3})
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO ${replacementFlags3}" )
	
	string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Z7")
	
	string(REGEX REPLACE "/Z[iI]" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
	string(REGEX REPLACE "/Z[iI]" "/Z7" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
endif()

if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "-undefined dynamic_lookup -pagezero_size 10000 -image_base 100000000")
endif()

if ("${CMAKE_SYSTEM}" MATCHES "Linux")
    set(CMAKE_EXE_LINKER_FLAGS "-rdynamic -Wl,-E")
endif()

include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/include"
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}"
        "${DEPS_DIR}")

if (WIN32)
    add_definitions(-DINA_OS_WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif (WIN32)

add_definitions(-DINA_OSTIME_ENABLED -DINA_TIME_DEFINED)

if (INAC_COVERAGE_ENABLED)
    message(STATUS "Coverage reports enabled")
    if(UNIX)
        find_program(GCOVR_PATH gcovr PATHS ${CMAKE_SOURCE_DIR}/scripts)

        if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
            message( WARNING "Code coverage results with an optimised (non-Debug) build may be misleading")
        endif()

        find_program(PYTHON_EXECUTABLE python)
        if(NOT PYTHON_EXECUTABLE)
            message(FATAL_ERROR "Python not found! Aborting...")
        endif()

        if(NOT GCOVR_PATH)
            message(FATAL_ERROR "gcovr not found! Aborting...")
        endif()

        set(COVERAGE_COMPILER_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage"
                CACHE INTERNAL "")

        set(CMAKE_CXX_FLAGS_COVERAGE
                ${COVERAGE_COMPILER_FLAGS}
                CACHE STRING "Flags used by the C++ compiler during coverage builds."
                FORCE )
        set(CMAKE_C_FLAGS_COVERAGE
                ${COVERAGE_COMPILER_FLAGS}
                CACHE STRING "Flags used by the C compiler during coverage builds."
                FORCE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
        message(STATUS "Appending code coverage compiler flags: ${COVERAGE_COMPILER_FLAGS}")
    endif()
    if(MSVC)
        find_program(OPENCPPCOVERAGE_PATH opencppcoverage.exe PATHS "C:/Program Files/OpenCppCoverage/")
        if(NOT OPENCPPCOVERAGE_PATH)
            message(FATAL_ERROR "OpenCppCoverage not found! Aborting...")
        endif()
    endif()

    set(COVERAGE_EXCLUDE "")
    if (EXISTS ${PROJECT_SOURCE_DIR}/tests/coverage.ignore)
        file(READ ${PROJECT_SOURCE_DIR}/tests/coverage.ignore CONTENT)
        string(REGEX REPLACE "\n" ";" CONTENT "${CONTENT}")
        foreach(LINE ${CONTENT})
            set(COVERAGE_EXCLUDE -e '${LINE}' ${COVERAGE_EXCLUDE})
        endforeach(LINE)
    endif()
endif()


function(inac_cmake_module NAME)
    cmake_parse_arguments(PARSE_ARGV 1 MOD "" URL "")
    if (NOT MOD_URL)
        set(MOD_URL "https://raw.githubusercontent.com/inaos/inac-cmake/${INAC_CMAKE_VERSION_MAJOR}.${INAC_CMAKE_VERSION_MINOR}/${NAME}.cmake")
    endif()
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/${NAME}.cmake")
        if (NOT EXISTS "${CMAKE_SOURCE_DIR}/${NAME}.cmake")
            message(STATUS "Downloading ${NAME}.cmake from ${MOD_URL}")
            file(DOWNLOAD "${MOD_URL}" "${CMAKE_BINARY_DIR}/${NAME}.cmake" STATUS DS)
            if(NOT "${DS}"  MATCHES "0;")
                file(REMOVE "${CMAKE_BINARY_DIR}/${NAME}.cmake")
                message(FATAL_ERROR "Failed to download ${NAME}.cmake ${DS}")
            endif()
        else()
            message(STATUS "Use local ${NAME}.cmake")
            configure_file("${CMAKE_SOURCE_DIR}/${NAME}.cmake" "${CMAKE_BINARY_DIR}/${NAME}.cmake" COPYONLY)
        endif()
    endif()
    include("${CMAKE_BINARY_DIR}/${NAME}.cmake")
endfunction()
    

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
    endif()
endfunction()

#
#
#
function(inac_enable_snapshot)
    set(INAC_SNAPSHOT ON PARENT_SCOPE)
endfunction()


#
# HEADER
#
function(inac_version_header HEADER)
    if (HEADER AND EXISTS ${CMAKE_SOURCE_DIR}/${HEADER}.in)
        message(STATUS "Version header ${HEADER}")
        configure_file(${CMAKE_SOURCE_DIR}/${HEADER}.in ${HEADER})
    endif()
    message(STATUS Major: ${PROJECT_VERSION_MAJOR})
    message(STATUS Minor: ${PROJECT_VERSION_MINOR})
    message(STATUS Patch: ${PROJECT_VERSION_PATCH})
endfunction()

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
    cmake_parse_arguments(PARSE_ARGV 1 LIB OMIT_PREFIX "CONFIGURE;DEPENDS;SOURCE_ROOT;COMMAND;COMMAND_ARGS;LIBNAME;ARCH;URL" "BUILD_TYPES")

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
            CONFIGURE_COMMAND "${LIB_CONFIGURE}"
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
    include_directories(${LIB_DIR})
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
                    "#include <libinac/lib.h>\nint main(int argc,  char** argv) { INA_MUST_SUCCEED(ina_app_init(argc, argv, NULL)); return ina_test_run(argc, argv, NULL);}"
                    )
            message(STATUS "Generate main.c for tests")
        endif ()
        list(APPEND src "${CMAKE_CURRENT_BINARY_DIR}/tests.dir/main.c")
    else ()
        list(APPEND src "${CMAKE_SOURCE_DIR}/tests/main.c")
        message(STATUS "Do NOT generate main.c for tests")
    endif ()
    add_executable(tests ${src})
    target_link_libraries(tests ${ARGN} ${INAC_DEPENDENCY_LIBS} ${PLATFORM_LIBS})
    inac_coverage(coverage tests tests-coverage "--format=junit>junit.xml")
    add_custom_target(runtests DEPENDS tests COMMAND ${CMD} "--format=junit>junit.xml" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
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
    file(GLOB src ${CMAKE_SOURCE_DIR}/bench/bench_*.c)
    list(LENGTH src src_count)
    if (${src_count} EQUAL 0)
        message(WARNING "Did no found any benchmark in ${CMAKE_SOURCE_DIR}/bench")
        return()
    endif ()
    message(STATUS "Found ${src_count} files to compile into bench")
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/bench/main.c")
        if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c")
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c
                    "#include <libinac/lib.h>\nint main(int argc,  char** argv) {  INA_MUST_SUCCEED(ina_app_init(argc, argv, NULL)); return ina_bench_run(argc, argv);}"
                    )
        endif ()
        list(APPEND src "${CMAKE_CURRENT_BINARY_DIR}/bench.dir/main.c")
    else ()
        list(APPEND src "${CMAKE_SOURCE_DIR}/bench/main.c")
        message(STATUS "Do NOT generate main.c for benchmarks")
    endif ()
    add_executable(bench ${src})
    target_link_libraries(bench ${ARGN} ${INAC_DEPENDENCY_LIBS} ${PLATFORM_LIBS})
    add_custom_target(runbenchmarks DEPENDS bench COMMAND "${CMD}" "--r=."  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    set_target_properties(runbenchmarks PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endfunction(inac_add_benchmarks)

#
#
#
function(inac_add_tools)
    remove_definitions(-DINA_LIB)
    set(tools "")
    file(GLOB src ${CMAKE_SOURCE_DIR}/tools/*.c)
    foreach (tool_src ${src})
        string(REGEX MATCH "^(.*)\\.[^.]*$" dummy ${tool_src})
        set(tool ${CMAKE_MATCH_1})
        STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/tools/" "" tool ${tool})
        add_executable(${tool} ${tool_src})
        target_link_libraries(${tool} ${ARGN} ${INAC_DEPENDENCY_LIBS} ${PLATFORM_LIBS})
        list(APPEND tools ${tool})
        message(STATUS "added tool ${tool}")
    endforeach ()
    set(INAC_TOOLS ${tools} PARENT_SCOPE)
endfunction(inac_add_tools)

#
#
#
function(inac_add_examples)
    remove_definitions(-DINA_LIB)
    set(examples "")
    file(GLOB src ${CMAKE_SOURCE_DIR}/examples/*.c)
    foreach (example_src ${src})
        string(REGEX MATCH "^(.*)\\.[^.]*$" dummy ${example_src})
        set(example ${CMAKE_MATCH_1})
        STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/examples/" "" example ${example})
        add_executable(${example} ${example_src})
        target_link_libraries(${example} ${ARGN} ${INAC_DEPENDENCY_LIBS} ${PLATFORM_LIBS})
        list(APPEND examples ${example})
        message(STATUS "added example ${example}")
    endforeach ()
    set(INAC_EXAMPLES ${examples} PARENT_SCOPE)
endfunction(inac_add_examples)

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
            "${PROJECT_SOURCE_DIR}/${TARGET}/${FILE}"
            $<TARGET_FILE_DIR:${TARGET}>/${CPY_DEST})
endfunction()

macro(inac_post_copy_file_win32 TARGET FILE)
    if (WIN32)
        inac_post_copy_file(${TARGET} ${FILE} ${ARGN})
    endif()
endmacro()

macro(inac_post_copy_file_unix TARGET FILE)
    if (UNIX)
        inac_post_copy_file(${TARGET} ${FILE} ${ARGN})
    endif()
endmacro()

macro(inac_post_copy_file_osx TARGET FILE)
    if (APPLE)
        inac_post_copy_file(${TARGET} ${FILE} ${ARGN})
    endif()
endmacro()

macro(inac_post_copy_file_linux TARGET FILE)
    if ("${CMAKE_SYSTEM}" MATCHES "Linux")
        inac_post_copy_file(${TARGET} ${FILE} ${ARGN})
    endif()
endmacro()


#
#
#
function(inac_add_contribs_headers)
    set(INAC_CONTRIBS_HEADERS "")
    foreach(file ${ARGN})
        message(STATUS "Include contrib header ${file}")
        string(CONCAT INAC_CONTRIBS_HEADERS ${INAC_CONTRIBS_HEADERS} "#include <libinac/contribs/" ${file} ">\n")
        configure_file(${DEPS_DIR}/${file} include/libinac/contribs/${file} COPYONLY)
    endforeach()
    configure_file(${CMAKE_SOURCE_DIR}/include/libinac/contribs.h.in include/libinac/contribs.h)
endfunction()
#
# Add lua file to compile
#
function(inac_add_luafiles TARGET)
    if(WIN32)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
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
    if(MSVC)
        file(WRITE ${SOURCE_FILE} "#pragma warning( disable : 4206)")
    endif()
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

function(inac_merge_static_libs outlib)
    set(libs ${ARGV})
    list(REMOVE_AT libs 0)
    # Create a dummy file that the target will depend on
    set(dummyfile ${outlib}_dummy.c)
    string(REPLACE "-" "_" dummyfile ${dummyfile})
    set(dummyfile ${CMAKE_CURRENT_BINARY_DIR}/${dummyfile})

    file(WRITE ${dummyfile} "const char * dummy = \"${dummyfile}\";")

    add_library(${outlib} STATIC ${dummyfile})

    # First get the file names of the libraries to be merged
    foreach(lib ${libs})
        get_target_property(libtype ${lib} TYPE)
        get_target_property(libfile ${lib} LOCATION)
        list(APPEND libfiles "${libfile}")
    endforeach()
    message(STATUS "will be merging ${libfiles}")

    list(REMOVE_DUPLICATES libfiles)

    # Now the easy part for MSVC and for MAC
    if(MSVC)
        set(LINKER_EXTRA_FLAGS "")
        foreach(l ${ARGN})
            get_property(LIB_LOCATION TARGET ${l} PROPERTY LOCATION)
            message(STATUS "Merge lib ${l}: ${LIB_LOCATION}")
            set(LINKER_EXTRA_FLAGS "${LINKER_EXTRA_FLAGS} \"${LIB_LOCATION}\"")
        endforeach()
        set_target_properties(${outlib} PROPERTIES STATIC_LIBRARY_FLAGS "${LINKER_EXTRA_FLAGS}")

    elseif(APPLE)
        get_target_property(outfile ${outlib} LOCATION)
        add_custom_command(TARGET ${outlib} POST_BUILD
                COMMAND rm ${outfile}
                COMMAND /usr/bin/libtool -static -o ${outfile}
                ${libfiles}
                )
    else()
        get_target_property(outfile ${outlib} LOCATION)
        message(STATUS "outfile location is ${outfile}")
        foreach(lib ${libfiles})
            # objlistfile will contain the list of object files for the library
            set(objlistfile ${CMAKE_BINARY_DIR}/${lib}.objlist)
            set(objdir ${CMAKE_BINARY_DIR}/${lib}.objdir)
            set(objlistcmake  ${CMAKE_BINARY_DIR}/${objlistfile}.cmake)
            # we only need to extract files once
            if(${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/cmake.check_cache IS_NEWER_THAN ${objlistcmake})
                #---------------------------------
                FILE(WRITE ${objlistcmake}
                        "# Extract object files from the library
message(STATUS \"Extracting object files from ${lib}\")
EXECUTE_PROCESS(COMMAND ${CMAKE_AR} -x ${lib}
                WORKING_DIRECTORY ${objdir})
# save the list of object files
EXECUTE_PROCESS(COMMAND ls .
				OUTPUT_FILE ${objlistfile}
                WORKING_DIRECTORY ${objdir})")
                #---------------------------------
                file(MAKE_DIRECTORY ${objdir})
                add_custom_command(
                        OUTPUT ${objlistfile}
                        COMMAND ${CMAKE_COMMAND} -P ${objlistcmake}
                        DEPENDS ${lib})
            endif()

            list(APPEND extrafiles "${objlistfile}")
            # relative path is needed by ar under MSYS
            file(RELATIVE_PATH objlistfilerpath ${objdir} ${objlistfile})
            add_custom_command(TARGET ${outlib} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo "Running: ${CMAKE_AR} ruU ${outfile} @${objlistfilerpath}"
                    COMMAND ${CMAKE_AR} ruU "${outfile}" @"${objlistfilerpath}"
                    WORKING_DIRECTORY ${objdir})
        endforeach()
        add_custom_command(TARGET ${outlib} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Running: ${CMAKE_RANLIB} ${outfile}"
                COMMAND ${CMAKE_RANLIB} ${outfile})
    endif()
    file(WRITE ${dummyfile}.base "const char* ${outlib}_sublibs=\"${libs}\";")
    add_custom_command(
            OUTPUT  ${dummyfile}
            COMMAND ${CMAKE_COMMAND}  -E copy ${dummyfile}.base ${dummyfile}
            DEPENDS ${libs} ${extrafiles})
endfunction()

function(inac_artifact_repository LOCAL)
    cmake_parse_arguments(PARSE_ARGV 2 R "" "REMOTE;USRPWD" "")

    if (NOT EXISTS "${INAC_REPOSITORY_PATH}")
        file(MAKE_DIRECTORY "${INAC_REPOSITORY_PATH}")
    endif()
    if (NOT EXISTS "${LOCAL}")
        file(MAKE_DIRECTORY "${LOCAL}")
        set(INAC_REPOSITORY_LOCAL "${LOCAL}" PARENT_SCOPE)
    endif()
    if (R_REMOTE AND NOT INAC_REPOSTORY_REMOTE)
        set(INAC_REPOSITORY_REMOTE "${R_REMOTE}" PARENT_SCOPE)
    endif()
    if (NOT INAC_REPOSITORY_USRPWD AND R_USRPWD)
        set(INAC_REPOSITORY_USRPWD "${R_USRPWD}" PARENT_SCOPE)
    endif()
endfunction()

function(inac_add_dependency name version )
    cmake_parse_arguments(PARSE_ARGV 2 DEP "SNAPSHOT" "REPOSITORY_REMOTE" "REPOSITORY_LOCAL")
    if (NOT DEP_REPOSITORY_REMOTE)
        set(DEP_REPOSITORY_REMOTE  ${INAC_REPOSITORY_REMOTE})
    endif()
        if (NOT DEP_REPOSITORY_LOCAL)
        set(DEP_REPOSITORY_LOCAL  ${INAC_REPOSITORY_LOCAL})
    endif()
    if (NOT DEP_REPOSITORY_LOCAL AND NOT DEP_REPOSITORY_REMOTE)
        message(FATAL_ERROR "local or remote repository must be given for dependency ${name}")
    endif()
    string(FIND ${version} "." patch_pos REVERSE)
    string(SUBSTRING ${version} 0 ${patch_pos} short_version)

    if (NOT DEP_SNAPSHOT)
        inac_artifact_name(${name} ${version} DEPENDENCY_NAME)
    else()
        inac_artifact_name(${name} ${short_version}.snapshot DEPENDENCY_NAME)
        string(REPLACE "release" "snapshot" DEP_REPOSITORY_REMOTE ${DEP_REPOSITORY_REMOTE})
    endif()

    set(LOCAL_PACKAGE_PATH "${DEP_REPOSITORY_LOCAL}/${DEPENDENCY_NAME}.zip")

    if (NOT EXISTS "${INAC_REPOSITORY_PATH}")
        file(MAKE_DIRECTORY "${INAC_REPOSITORY_PATH}")
    endif()

    if (NOT EXISTS "${INAC_REPOSITORY_PATH}/${DEPENDENCY_NAME}" OR DEP_SNAPSHOT)
        if(EXISTS "${LOCAL_PACKAGE_PATH}" AND (NOT DEP_SNAPSHOT))
            message(STATUS "Dependency ${DEPENDENCY_NAME} found in local repository ${DEP_REPOSITORY_LOCAL}")
            file(COPY "${LOCAL_PACKAGE_PATH}" DESTINATION "${INAC_REPOSITORY_PATH}")
        else()
            message(STATUS "Dependency ${DEPENDENCY_NAME} from ${DEP_REPOSITORY_URL}")
            if (INAC_REPOSITORY_USRPWD)
                file(DOWNLOAD "${DEP_REPOSITORY_REMOTE}/${name}/${short_version}/${DEPENDENCY_NAME}.zip" "${LOCAL_PACKAGE_PATH}" STATUS DS USERPWD ${INAC_REPOSITORY_USRPWD} LOG DL)
            else()
                file(DOWNLOAD "${DEP_REPOSITORY_REMOTE}/${name}/${short_version}/${DEPENDENCY_NAME}.zip" "${LOCAL_PACKAGE_PATH}" STATUS DS LOG DL)
            endif()
            if(NOT "${DS}"  MATCHES "0;")
                file(REMOVE "${LOCAL_PACKAGE_PATH}")
                message(FATAL_ERROR "Failed to download dependency ${DEPENDENCY_NAME} from ${DEP_REPOSITORY_REMOTE}: ${DL}")
            endif()
        endif()
        file(COPY "${LOCAL_PACKAGE_PATH}" DESTINATION "${INAC_REPOSITORY_PATH}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory "${INAC_REPOSITORY_PATH}/${DEPENDENCY_NAME}"
                WORKING_DIRECTORY "${INAC_REPOSITORY_PATH}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${INAC_REPOSITORY_PATH}/${DEPENDENCY_NAME}.zip"
                WORKING_DIRECTORY "${INAC_REPOSITORY_PATH}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E remove  "${INAC_REPOSITORY_PATH}/${DEPENDENCY_NAME}.zip"
                WORKING_DIRECTORY "${INAC_REPOSITORY_PATH}")
    endif()
    include_directories("${INAC_REPOSITORY_PATH}/${DEPENDENCY_NAME}/include")
    set(deps ${INAC_DEPENDENCY_LIBS})
    file(GLOB libs "${INAC_REPOSITORY_PATH}/${DEPENDENCY_NAME}/lib/*")
    foreach(lib ${libs})
        list(APPEND deps "${lib}")
    endforeach()
    set(INAC_DEPENDENCY_LIBS ${deps} PARENT_SCOPE)
    message(STATUS "Add binary dependency ${name}: ${DEPENDENCY_NAME}")
endfunction()


macro(inac_check_arch arch)
    set(ARCHS "armv7;armv6;armv5;arm;x86;x86_64;ia64;ppc64;ppc;ppc64")
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
    #error cmake_ARCH x86
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
            list(APPEND ARCH x86)
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

function (inac_package)
    cmake_parse_arguments(P "" "NAME;VENDOR;SUMMARY;INSTALL_DIRECTORY" "")
    set(CPACK_GENERATOR ZIP)
    if (P_NAME)
        set(CPACK_PACKAGE_NAME ${P_NAME})
    else()
        set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
    endif()
    if(P_VENDOR)
        set(CPACK_PACKAGE_VENDOR ${P_VENDOR})
    endif()
    if (P_SUMMARY)
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${P_SUMMARY})
    endif()
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    if (NOT INAC_SNAPSHOT)
        set(version "${CPACK_PACKAGE_VERSION}")
    else()
        set(version "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.snapshot")
    endif()
    inac_artifact_name("${CPACK_PACKAGE_NAME}" "${version}" CPACK_PACKAGE_FILE_NAME)
    include(CPack)
    install(TARGETS ${INAC_EXAMPLES}
            DESTINATION examples
            COMPONENT binaries)
    install(TARGETS ${INAC_TOOLS}
            DESTINATION bin
            COMPONENT binaries)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/lib${CMAKE_PROJECT_NAME}
            DESTINATION include
            FILES_MATCHING
            PATTERN *.h)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/lib${CMAKE_PROJECT_NAME}
            DESTINATION include
            FILES_MATCHING
            PATTERN *.h)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/doc/
            DESTINATION doc
            FILES_MATCHING
            PATTERN *.md)
endfunction()

function (inac_load_config_file PATH REQUIRED)
    if(EXISTS "${PATH}")
        file(STRINGS "${PATH}" contents)
        foreach(NameAndValue ${contents})
            string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
            string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
            string(REPLACE "${Name}=" "" Value ${NameAndValue})
            set(${Name} "${Value}" PARENT_SCOPE)
        endforeach()
    else()
        if (REQUIRED)
            message(FATAL_ERROR "Config file ${PATH} cannot be read")
        endif()
    endif()
endfunction()

function(inac_artifact_name name version output_var)
    if (MSVC)
        if ($ENV{VisualStudioVersion} STREQUAL "12.0")
            string(APPEND ARTIFACT_NAME "${name}-${CMAKE_SYSTEM_NAME}_vs13-${INAC_TARGET_ARCH}-${CMAKE_BUILD_TYPE}-${version}")
        elseif($ENV{VisualStudioVersion} STREQUAL "14.0")
            string(APPEND ARTIFACT_NAME "${name}-${CMAKE_SYSTEM_NAME}_vs15-${INAC_TARGET_ARCH}-${CMAKE_BUILD_TYPE}-${version}")
        elseif($ENV{VisualStudioVersion} STREQUAL "15.0")
            string(APPEND ARTIFACT_NAME "${name}-${CMAKE_SYSTEM_NAME}_vs17-${INAC_TARGET_ARCH}-${CMAKE_BUILD_TYPE}-${version}")
        else()
            message(FATAL_ERROR "Unknown Visual-Studio version: $ENV{VisualStudioVersion}")
        endif()
    else()
        string(APPEND ARTIFACT_NAME "${name}-${CMAKE_SYSTEM_NAME}-${INAC_TARGET_ARCH}-${CMAKE_BUILD_TYPE}-${version}")
    endif()
    string(TOLOWER ${ARTIFACT_NAME} ARTIFACT_NAME)
    set("${output_var}" ${ARTIFACT_NAME} PARENT_SCOPE)
endfunction()


function(inac_coverage TARGET RUNNER OUTPUT)
    if(INAC_COVERAGE_ENABLED)
        if(UNIX)
            TARGET_LINK_LIBRARIES(${RUNNER} gcov)
            set_target_properties(${RUNNER} PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
            ADD_CUSTOM_TARGET(${TARGET}
                    ${RUNNER} ${ARGV3} || (exit 0)
                    COMMAND ${GCOVR_PATH} -x -r ${CMAKE_SOURCE_DIR} -o ${OUTPUT}.xml --filter="${CMAKE_SOURCE_DIR}/src/" --filter="${CMAKE_SOURCE_DIR}/include/" ${COVERAGE_EXCLUDE} ${ARGV4}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMAND xsltproc c2s.xsl ${OUTPUT}.xml > ${OUTPUT}.sonar.xml
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Running gcovr to produce code coverage report."
                    )
        endif()
        if(MSVC)
            file(TO_NATIVE_PATH ${CMAKE_SOURCE_DIR}/include COV_INC_PATH)
            file(TO_NATIVE_PATH ${CMAKE_SOURCE_DIR}/src COV_SRC_PATH)
            ADD_CUSTOM_TARGET(${TARGET}
                    COMMAND ${OPENCPPCOVERAGE_PATH} --working_dir=${CMAKE_BINARY_DIR} --sources=${COV_INC_PATH} --sources=${COV_SRC_PATH} ${COVERAGE_EXCLUDE} --export_type=cobertura:${OUTPUT}.xml -- ${RUNNER}.exe ${ARGV3} & exit 0
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMAND msxsl.exe  ${OUTPUT}.xml c2s.xsl -o ${OUTPUT}.sonar.xml source=${CMAKE_SOURCE_DIR}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Running OppCppCoverage to produce code coverage report.")
            ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD
                    COMMAND ;
                    COMMENT "Cobertura code coverage report saved in ${OUTPUT}.xml."
                    )
        endif()
    endif()
endfunction()

inac_detect_host_arch()
if (NOT INAC_TARGET_ARCH)
    inac_set_target_arch(${INAC_HOST_ARCH})
else()
    message(STATUS "Target architecture: ${INAC_TARGET_ARCH}")
endif()

if (NOT INAC_REPOSITORY)
    set(INAC_REPOSITORY repository)
endif()



if (MSVC)
set(INAC_C2S "<?xml version=\"1.0\" ?>\r\n<xsl:transform xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" version=\"1.0\">\r\n  <xsl:param name=\"source\" select=\"source\"/>\r\n <xsl:variable name=\"src-matcher\" select=\"substring($source, 2)\"/>\r\n  <xsl:output indent=\"yes\" omit-xml-declaration=\"no\" />\r\n\r\n    <xsl:variable name=\"TAB\">\r\n        <xsl:text>&#32;&#32;&#32;&#32;</xsl:text>\r\n    </xsl:variable>\r\n    <xsl:variable name=\"CR\">\r\n        <xsl:text>&#xA;</xsl:text>\r\n    </xsl:variable>\r\n    <xsl:param name=\"SRC_DIR\"/>\r\n\r\n    <xsl:template match=\"/coverage\">\r\n        <xsl:value-of select=\"$CR\" />\r\n        <coverage version=\"1\">\r\n            <xsl:value-of select=\"$CR\" />\r\n            <xsl:apply-templates mode=\"custom-copy\" select=\".\" />\r\n        </coverage>\r\n    </xsl:template>\r\n\r\n    <xsl:template mode=\"custom-copy\" match=\"/coverage/packages/package/classes\">\r\n        <xsl:for-each select=\"class\">\r\n            <xsl:variable name=\"currFilename\" select=\"@filename\" />\r\n            <xsl:if test=\". = /coverage/packages/package/classes/class[@filename=$currFilename][1]\">\r\n                <xsl:value-of select=\"$TAB\" />\r\n                <file path=\"{substring(@filename, string-length($src-matcher))}\">\r\n                    <xsl:value-of select=\"$CR\" />\r\n                    <xsl:for-each select=\"/coverage/packages/package/classes/class[@filename=$currFilename]\">\r\n                        <xsl:for-each select=\"lines/line\">\r\n                            <xsl:apply-templates mode=\"custom-copy\" select=\".\" />\r\n                        </xsl:for-each>\r\n                    </xsl:for-each>\r\n                    <xsl:value-of select=\"$TAB\" />\r\n                </file>\r\n                <xsl:value-of select=\"$CR\" />\r\n            </xsl:if>\r\n        </xsl:for-each>\r\n    </xsl:template>\r\n\r\n    <xsl:template mode=\"custom-copy\" match=\"/coverage/packages/package/classes/class/lines/line\">\r\n        <xsl:value-of select=\"$TAB\" />\r\n        <xsl:value-of select=\"$TAB\" />\r\n        <xsl:choose>\r\n            <xsl:when test=\"@branch='true'\">\r\n                <xsl:variable name=\"COVERAGE_SEPARATOR\"><![CDATA[/]]></xsl:variable>\r\n                <xsl:variable name=\"COVERAGE\" select=\"translate(translate(substring-after(normalize-space(@condition-coverage), '% '), ')', ''), '(', '')\" />\r\n                <lineToCover lineNumber=\"{@number}\" covered=\"{boolean(@hits &gt; 0)}\" branchesToCover=\"{substring-after($COVERAGE, $COVERAGE_SEPARATOR)}\" coveredBranches=\"{substring-before($COVERAGE, $COVERAGE_SEPARATOR)}\" />\r\n            </xsl:when>\r\n            <xsl:otherwise>\r\n                <lineToCover lineNumber=\"{@number}\" covered=\"{boolean(@hits &gt; 0)}\" />\r\n            </xsl:otherwise>\r\n        </xsl:choose>\r\n        <xsl:value-of select=\"$CR\" />\r\n    </xsl:template>\r\n\r\n    <xsl:template mode=\"custom-copy\" match=\"@* | node()\">\r\n        <xsl:apply-templates mode=\"custom-copy\" select=\"@* | node()\" />\r\n    </xsl:template>\r\n</xsl:transform>")
else()
set(INAC_C2S "<?xml version=\"1.0\" ?>\r\n<xsl:transform xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" version=\"1.0\">\r\n    <xsl:output indent=\"yes\" omit-xml-declaration=\"no\" />\r\n\r\n    <xsl:variable name=\"TAB\">\r\n        <xsl:text>&#32;&#32;&#32;&#32;</xsl:text>\r\n    </xsl:variable>\r\n    <xsl:variable name=\"CR\">\r\n        <xsl:text>&#xA;</xsl:text>\r\n    </xsl:variable>\r\n    <xsl:param name=\"SRC_DIR\"/>\r\n\r\n    <xsl:template match=\"/coverage\">\r\n        <xsl:value-of select=\"$CR\" />\r\n        <coverage version=\"1\">\r\n            <xsl:value-of select=\"$CR\" />\r\n            <xsl:apply-templates mode=\"custom-copy\" select=\".\" />\r\n        </coverage>\r\n    </xsl:template>\r\n\r\n    <xsl:template mode=\"custom-copy\" match=\"/coverage/packages/package/classes\">\r\n        <xsl:for-each select=\"class\">\r\n            <xsl:variable name=\"currFilename\" select=\"@filename\" />\r\n            <xsl:if test=\". = /coverage/packages/package/classes/class[@filename=$currFilename][1]\">\r\n                <xsl:value-of select=\"$TAB\" />\r\n                <file path=\"{$SRC_DIR}{@filename}\">\r\n                    <xsl:value-of select=\"$CR\" />\r\n                    <xsl:for-each select=\"/coverage/packages/package/classes/class[@filename=$currFilename]\">\r\n                        <xsl:for-each select=\"lines/line\">\r\n                            <xsl:apply-templates mode=\"custom-copy\" select=\".\" />\r\n                        </xsl:for-each>\r\n                    </xsl:for-each>\r\n                    <xsl:value-of select=\"$TAB\" />\r\n                </file>\r\n                <xsl:value-of select=\"$CR\" />\r\n            </xsl:if>\r\n        </xsl:for-each>\r\n    </xsl:template>\r\n\r\n    <xsl:template mode=\"custom-copy\" match=\"/coverage/packages/package/classes/class/lines/line\">\r\n        <xsl:value-of select=\"$TAB\" />\r\n        <xsl:value-of select=\"$TAB\" />\r\n        <xsl:choose>\r\n            <xsl:when test=\"@branch='true'\">\r\n                <xsl:variable name=\"COVERAGE_SEPARATOR\"><![CDATA[/]]></xsl:variable>\r\n                <xsl:variable name=\"COVERAGE\" select=\"translate(translate(substring-after(normalize-space(@condition-coverage), '% '), ')', ''), '(', '')\" />\r\n                <lineToCover lineNumber=\"{@number}\" covered=\"{boolean(@hits &gt; 0)}\" branchesToCover=\"{substring-after($COVERAGE, $COVERAGE_SEPARATOR)}\" coveredBranches=\"{substring-before($COVERAGE, $COVERAGE_SEPARATOR)}\" />\r\n            </xsl:when>\r\n            <xsl:otherwise>\r\n                <lineToCover lineNumber=\"{@number}\" covered=\"{boolean(@hits &gt; 0)}\" />\r\n            </xsl:otherwise>\r\n        </xsl:choose>\r\n        <xsl:value-of select=\"$CR\" />\r\n    </xsl:template>\r\n\r\n    <xsl:template mode=\"custom-copy\" match=\"@* | node()\">\r\n        <xsl:apply-templates mode=\"custom-copy\" select=\"@* | node()\" />\r\n    </xsl:template>\r\n</xsl:transform>")
endif()
file(WRITE "${CMAKE_BINARY_DIR}/c2s.xsl" "${INAC_C2S}")

inac_load_config_file("${INAC_REPOSITORY_PATH}/${INAC_REPOSITORY}.txt" FALSE)
inac_enable_trace(Debug 1)
inac_enable_log(Debug 4)
inac_enable_log(RelWithDebInfo 3)
inac_enable_log(Release 3)
inac_platform_libs_for_win("Ws2_32.lib;Psapi.lib;Iphlpapi.lib;winmm.lib;DbgHelp.lib")
inac_platform_libs_for_linux("-lrt -ldl -lm")
inac_platform_libs_for_osx("-ldl -lm")

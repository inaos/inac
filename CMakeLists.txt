cmake_minimum_required (VERSION 2.6)
project (inac)

set (DEPS_DIR "${CMAKE_SOURCE_DIR}/contribs")
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src")

set (ISTDLIB_INCLUDE_DIR "${DEPS_DIR}/istdlib/include")
set (UV_INCLUDE_DIR "${DEPS_DIR}/libuv/include")

if(WIN32)
set (ISTDLIB_LIBRARIES "${DEPS_DIR}/istdlib/build/debug/istdlib.lib")
set (UV_LIBRARIES "${DEPS_DIR}/libuv/Debug/lib/uv.lib")
endif(WIN32)

include_directories("${PROJECT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/include" "${ISTDLIB_INCLUDE_DIR}"
                    "${UV_INCLUDE_DIR}")

#if(WIN32)
#	set (PLATFORM_LIBS Ws2_32.lib Psapi.lib Iphlpapi.lib)
#endif(WIN32)

# Memory
add_library(memory ${SRC_DIR}/memory.c)

# Memory Test
add_executable(test_memory ${SRC_DIR}/test_mempool.c)
target_link_libraries(test_memory memory)




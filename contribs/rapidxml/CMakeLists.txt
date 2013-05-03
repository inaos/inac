cmake_minimum_required (VERSION 2.6)
project (rapidxml)

set (SRC_DIR "${CMAKE_SOURCE_DIR}")

string( TOLOWER "$ENV{INAC_W32_BUILD_TYPE}" INA_BUILD_TYPE)
MESSAGE(STATUS "Build-Type: ${INA_BUILD_TYPE}")

# BEGIN PREPROCESSOR SECTION

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif(WIN32)

# END PREPROCESSOR SECTION

add_library(inac-rapidxml SHARED ${SRC_DIR}/rapidxml.cpp)
target_link_libraries(inac-rapidxml)

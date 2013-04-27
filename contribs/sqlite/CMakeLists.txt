cmake_minimum_required (VERSION 2.6)
project (sqlite3)

#SQLITE3 static lib
set (SRC sqlite3.c)
add_library(sqlite ${SRC})

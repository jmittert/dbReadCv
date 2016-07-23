cmake_minimum_required(VERSION 2.8)
project( DBDisplay )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package( OpenCV REQUIRED )
find_package( PostgreSQL REQUIRED )
find_package( LibPQXX REQUIRED )

if(NOT POSTGRESQL_FOUND)
  message(FATAL_ERROR " Please check your PostgreSQL installation.") 
endif(NOT POSTGRESQL_FOUND)

if(NOT LIBPQXX_FOUND)
  message(FATAL_ERROR " Please check your libqp installation.") 
endif(NOT LIBPQXX_FOUND)

add_executable( DBDisplay src/read.cpp )
add_executable( DBNN src/nnw.cpp)

include_directories(${PostgreSQL_INCLUDE_DIR} ${LIBPQXX_INCLUDE_DIRS})
target_link_libraries( DBDisplay ${OpenCV_LIBS} ${PostgreSQL_LIBRARIES} /usr/lib/libpqxx.so)
target_link_libraries( DBNN ${OpenCV_LIBS} ${PostgreSQL_LIBRARIES} /usr/lib/libpqxx.so)
LINK_DIRECTORIES(${LIBPQXX_LIBRARY_DIRS})

set (CMAKE_CXX_FLAGS)

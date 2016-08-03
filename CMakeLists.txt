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

add_executable( train src/main/nnw.cpp src/lib/lib.cpp src/lib/features.cpp)
add_executable( control src/main/control.cpp src/lib/lib.cpp src/lib/features.cpp)

include_directories(${PostgreSQL_INCLUDE_DIR} ${LIBPQXX_INCLUDE_DIRS} include)
target_link_libraries( train ${OpenCV_LIBS} ${PostgreSQL_LIBRARIES} /usr/lib/libpqxx.so)
target_link_libraries( control ${OpenCV_LIBS} ${PostgreSQL_LIBRARIES} /usr/lib/libpqxx.so)
LINK_DIRECTORIES(${LIBPQXX_LIBRARY_DIRS})

set (CMAKE_CXX_FLAGS)

cmake_minimum_required(VERSION 2.8)
project( DBDisplay )

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package( OpenCV REQUIRED )
find_package( PostgreSQL REQUIRED )
find_package( LibPQXX REQUIRED )
enable_testing()

file(GLOB test_SRC "test/*.cpp")

if(NOT POSTGRESQL_FOUND)
  message(FATAL_ERROR " Please check your PostgreSQL installation.") 
endif(NOT POSTGRESQL_FOUND)

if(NOT LIBPQXX_FOUND)
  message(FATAL_ERROR " Please check your libqp installation.") 
endif(NOT LIBPQXX_FOUND)

add_executable( train src/main/nnw.cpp src/lib/lib.cpp src/lib/features.cpp src/lib/cvlib.cpp)
add_executable( ncontrol src/main/nncontrol.cpp src/lib/lib.cpp src/lib/features.cpp src/lib/cvlib.cpp)
add_executable( watch src/main/watch.cpp src/lib/lib.cpp src/lib/features.cpp src/lib/cvlib.cpp)
add_executable( gcontrol src/main/gpcontrol.cpp src/lib/car.cpp src/lib/gamepad.cpp src/lib/server.cpp src/lib/communicator.cpp)
add_executable( car src/main/carClient.cpp src/lib/car.cpp src/lib/client.cpp src/lib/communicator.cpp src/lib/lib.cpp)
add_executable( testExe ${test_SRC} src/lib/client.cpp src/lib/server.cpp src/lib/communicator.cpp src/lib/lib.cpp)

include_directories(${PostgreSQL_INCLUDE_DIR} ${LIBPQXX_INCLUDE_DIRS} include)
target_link_libraries( train ${OpenCV_LIBS} ${PostgreSQL_LIBRARIES} /usr/lib/libpqxx.so)
target_link_libraries( ncontrol ${OpenCV_LIBS} ${PostgreSQL_LIBRARIES} /usr/lib/libpqxx.so)
target_link_libraries( watch ${OpenCV_LIBS} ${PostgreSQL_LIBRARIES} /usr/lib/libpqxx.so)
target_link_libraries( gcontrol /usr/lib/libwiringPi.so pthread)
target_link_libraries( car /usr/lib/libwiringPi.so ${PostgreSQL_LIBRARIES} /usr/lib/libpqxx.so )

target_link_libraries( testExe UnitTest++)
add_test( testAll ./testExe )
LINK_DIRECTORIES(${LIBPQXX_LIBRARY_DIRS})

set (CMAKE_CXX_FLAGS "-std=c++11")

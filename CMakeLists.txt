cmake_minimum_required(VERSION 3.10)
project(ARChessCube)
find_package(OpenCV REQUIRED)

include_directories(include)

file( GLOB SOURCES src/*.cpp)

add_executable(ARChessCube ${SOURCES} )
target_link_libraries(ARChessCube PRIVATE ${OpenCV_LIBS} )
target_include_directories(ARChessCube PRIVATE ${OpenCV_INCLUDE_DIRS})
cmake_minimum_required(VERSION 3.10)
project(ARChessCube)

find_package(OpenCV REQUIRED)

# Вместо find_package(nlohmann_json) просто включаем заголовочный файл
include_directories(/usr/include)

include_directories(include)

file(GLOB SOURCES src/*.cpp)

add_executable(ARChessCube ${SOURCES})
target_link_libraries(ARChessCube PRIVATE ${OpenCV_LIBS})
cmake_minimum_required(VERSION 3.10)
project(adas_overpass_csv)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(adas_overpass src/main.cpp)
target_include_directories(adas_overpass PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(adas_overpass PRIVATE ${CURL_LIBRARIES} nlohmann_json::nlohmann_json)

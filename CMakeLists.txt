cmake_minimum_required(VERSION 3.19)
project(untitled)

set(CMAKE_CXX_STANDARD 20)
INCLUDE(FindPkgConfig)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
# Searching SDL2 package.
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
find_package(pugixml REQUIRED)

# Including our external library's directory for header files!
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})
add_executable(untitled main.cpp)
target_link_libraries(untitled ${SDL2_LIBRARIES} pugixml)

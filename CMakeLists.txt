cmake_minimum_required(VERSION 3.0)

project(osuds-res-conv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wno-narrowing -O3")

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -I/usr/local/Cellar/sdl2/2.0.20/include -I/usr/local/Cellar/sdl2_image/2.0.5/include -L/usr/local/Cellar/sdl2/2.0.20/lib -L/usr/local/Cellar/sdl2_image/2.0.5/lib")

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} SDL2_image SDL2)
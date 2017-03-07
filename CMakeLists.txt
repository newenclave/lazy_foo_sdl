cmake_minimum_required(VERSION 2.8)

set( PROJECT_NAME lazyfoosdl )

set( CMAKE_CXX_STANDARD_REQUIRED 11 )
set( CMAKE_CXX_STANDARD 11 )

project( ${PROJECT_NAME} )

find_package( SDL2 REQUIRED )

message(STATUS "Include directory: " ${SDL2_INCLUDE_DIRS} )
message(STATUS "Libraries: "         ${SDL2_LIBRARIES} )

add_executable( ${PROJECT_NAME} "main.cpp" )

include_directories( ${SDL2_INCLUDE_DIRS} )

target_link_libraries( ${PROJECT_NAME} SDL2 SDL2_image )



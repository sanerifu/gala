cmake_minimum_required(VERSION 3.10)

project(gala C)


add_library(glad libraries/glad/src/glad.c)
target_include_directories(glad PUBLIC libraries/glad/include)

set(SOURCES
source/gala.h source/gala.c
)

add_library(gala ${SOURCES})
target_link_libraries(gala PRIVATE glad)
target_include_directories(gala PUBLIC ${CMAKE_SOURCE_DIR} "$<TARGET_PROPERTY:glad,INTERFACE_INCLUDE_DIRECTORIES>")

find_package(glfw3 3.4 REQUIRED)
add_executable(gala_exe source/exe.c)
target_link_libraries(gala_exe PRIVATE gala glfw)

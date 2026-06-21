cmake_minimum_required(VERSION 3.10)

project(gala C)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
add_compile_options("-ffile-prefix-map=${CMAKE_SOURCE_DIR}/=")
elseif(MSVC)
add_compile_options("/FC" "/Zc:wchar_t")
endif()


add_library(glad libraries/glad/src/glad.c)
target_include_directories(glad PUBLIC libraries/glad/include)

set(SOURCES
source/gala/gala.h source/gala/gala.c
)

add_library(gala ${SOURCES})
target_link_libraries(gala PRIVATE glad)
target_include_directories(gala PUBLIC ${CMAKE_SOURCE_DIR} "$<TARGET_PROPERTY:glad,INTERFACE_INCLUDE_DIRECTORIES>")

find_package(glfw3 3.3 REQUIRED)
add_executable(gala_exe source/exe.c)
target_link_libraries(gala_exe PRIVATE gala glfw)
target_include_directories(gala_exe PRIVATE "$<TARGET_PROPERTY:gala,INTERFACE_INCLUDE_DIRECTORIES>")

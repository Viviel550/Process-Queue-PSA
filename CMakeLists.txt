cmake_minimum_required(VERSION 3.10)
project(QueueSimulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(sim ${SOURCES})
target_include_directories(sim PRIVATE src)
target_compile_options(sim PRIVATE -Wall -Wextra)

find_package(Threads REQUIRED)
target_link_libraries(sim PRIVATE Threads::Threads)

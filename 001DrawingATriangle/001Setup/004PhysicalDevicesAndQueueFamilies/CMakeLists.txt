cmake_minimum_required(VERSION 2.8)
project(physicsal_devices_and_queue_families C CXX) # Project Name
set(CMAKE_CXX_STANDARD 17) # -std=c++17

find_package(Vulkan)
include_directories(${Vulkan_INCLUDE_DIR})
set(ALL_LIBS  ${Vulkan_LIBRARY} glfw )

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall") # C option 
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall") # CXX option

add_executable(physicsal_devices_and_queue_families main.cc) # binary_name source source ...
target_link_libraries(physicsal_devices_and_queue_families ${ALL_LIBS}) # Link Libraries

cmake_minimum_required(VERSION 3.8)

project(vulkan-app C CXX) # Project Name

option(VULKAN_APP_USE_GLOG "Use glog" OFF)

# find package
find_package(glfw3 REQUIRED)
find_package(Vulkan)

add_executable(${PROJECT_NAME} main.cc) # binary_name source source ...

target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARY} glfw) # Link Libraries

if(${VULKAN_APP_USE_GLOG})
  find_package(glog 0.5.0 REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE glog::glog)
  target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GLOG)
endif(${VULKAN_APP_USE_GLOG})

target_compile_options(
  ${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-global-constructors>
  $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-global-constructors>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wredundant-decls -Wcast-align -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Winvalid-pch -Wredundant-decls -Wformat=2 -Wmissing-format-attribute -Wformat-nonliteral>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

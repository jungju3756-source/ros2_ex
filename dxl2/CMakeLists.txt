cmake_minimum_required(VERSION 3.5)
project(dxl2)
# Default to C99
if(NOT CMAKE_C_STANDARD)
set(CMAKE_C_STANDARD 99)
endif()
# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
set(CMAKE_CXX_STANDARD 14)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
add_compile_options(-Wall -Wextra -Wpedantic)
endif()
# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
include_directories(include)
include_directories(/usr/local/include/dynamixel_sdk)
# Build
add_executable(dxl2 src/main.cpp src/dxl.cpp)
ament_target_dependencies(dxl2 rclcpp)
target_link_libraries(dxl2 dxl_x64_cpp)
#target_include_directories(sub PUBLIC /usr/local/include/dynamixel_sdk)
install(TARGETS
dxl2
DESTINATION lib/${PROJECT_NAME})
ament_package()
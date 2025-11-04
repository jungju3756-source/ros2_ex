cmake_minimum_required(VERSION 3.16)
project(pub1_1)

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
find_package(std_msgs REQUIRED)

# build
add_executable(pub1_1 src/pub1_1.cpp)
ament_target_dependencies(pub1_1 rclcpp std_msgs)

# install
install(TARGETS
  pub1_1
  DESTINATION lib/${PROJECT_NAME})

ament_package()


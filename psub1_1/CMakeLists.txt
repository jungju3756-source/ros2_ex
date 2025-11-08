cmake_minimum_required(VERSION 3.16)
project(psub1_1)
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
#build
add_executable(pub1_1 src/pub1_1.cpp)
ament_target_dependencies(pub1_1 rclcpp std_msgs)
add_executable(sub1_1 src/sub1_1.cpp)
ament_target_dependencies(sub1_1 rclcpp std_msgs)
# Install
install(TARGETS
pub1_1
sub1_1
DESTINATION lib/${PROJECT_NAME})
ament_package()
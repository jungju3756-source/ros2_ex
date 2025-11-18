cmake_minimum_required(VERSION 3.16)
project(camera2_3)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(cv_bridge REQUIRED)

add_executable(pub src/pub.cpp src/pmain.cpp)
ament_target_dependencies(pub rclcpp sensor_msgs cv_bridge OpenCV)

add_executable(sub src/sub.cpp src/smain.cpp)
ament_target_dependencies(sub rclcpp sensor_msgs cv_bridge OpenCV)

install(TARGETS pub sub DESTINATION lib/${PROJECT_NAME})

ament_package()

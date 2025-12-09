cmake_minimum_required(VERSION 3.16)
project(linedetect_wsl_2)

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
find_package(std_msgs REQUIRED)     # pub.cpp에서 std_msgs::msg::Header 사용
find_package(sensor_msgs REQUIRED)  # CompressedImage 메시지 사용
find_package(cv_bridge REQUIRED)    # cv_bridge 사용
find_package(OpenCV REQUIRED)       # OpenCV 라이브러리 사용

# build - 퍼블리셔 노드 (campub_node)
add_executable(campub_node
    src/pmain.cpp  # 메인 함수
    src/pub.cpp    # CamPublisher 구현체
)
ament_target_dependencies(campub_node
    rclcpp
    sensor_msgs
    std_msgs
    cv_bridge
)
# OpenCV 라이브러리 링크와 인클루드 경로 추가
target_include_directories(campub_node PRIVATE
    include
)
target_link_libraries(campub_node
    ${OpenCV_LIBS}
)


# build - 서브스크라이버 노드 (camsub_node)
add_executable(camsub_node
    src/smain.cpp  # 메인 함수
    src/sub.cpp    # LineSubscriber 구현체
)
ament_target_dependencies(camsub_node
    rclcpp
    sensor_msgs
)
# OpenCV 라이브러리 링크와 인클루드 경로 추가
target_include_directories(camsub_node PRIVATE
    include
)
target_link_libraries(camsub_node
    ${OpenCV_LIBS}
)


# Install
install(TARGETS
    campub_node
    camsub_node
    DESTINATION lib/${PROJECT_NAME}
)

# 헤더 파일 설치 (다른 패키지에서 사용할 수 있도록)
install(DIRECTORY include/
    DESTINATION include
)

ament_package()
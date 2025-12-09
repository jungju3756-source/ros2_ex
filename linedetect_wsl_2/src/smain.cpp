#include "linedetect_wsl_2/sub.hpp"

int main(int argc, char *argv[])
{
    // ROS2 노드 초기화
    rclcpp::init(argc, argv);
    
    // LineSubscriber 노드 생성 및 실행 (스핀)
    // 메시지가 들어올 때마다 콜백 함수가 호출됨
    rclcpp::spin(std::make_shared<LineSubscriber>());
    
    // 종료
    rclcpp::shutdown();
    return 0;
}

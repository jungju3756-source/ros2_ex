#include "pub.hpp"

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CameraPublisher>();
    node->publish_image();
    rclcpp::shutdown();
    return 0;
}

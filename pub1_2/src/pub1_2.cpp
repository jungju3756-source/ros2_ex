#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include <memory>
#include <chrono>
#include <iostream>
#include <limits>
int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("node_pub1_2");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    auto mypub = node->create_publisher<geometry_msgs::msg::Vector3>("topic_pub1_2",
    qos_profile );
    geometry_msgs::msg::Vector3 message;
    // 콘솔에서 x, y, z 값을 입력받아 설정
    double x, y, z;
    std::cout << "보낼 Vector3 값 입력 (x y z): " << std::flush;
    while(!(std::cin >> x >> y >> z))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "잘못된 입력입니다. 다시 입력 (x y z): " << std::flush;
    }
    message.x = x;
    message.y = y;
    message.z = z;
    rclcpp::WallRate loop_rate(1.0); 
    while(rclcpp::ok())
    {
        RCLCPP_INFO(node->get_logger(), "Publish: %f, %f, %f", message.x, message.y, message.z);
        mypub->publish(message);
        //rclcpp::spin_some(node);
        loop_rate.sleep(); //반복주파수에서 남은 시간 만큼 sleep
    }
    rclcpp::shutdown();
    return 0;
}


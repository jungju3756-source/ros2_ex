#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"
#include <memory>
#include <chrono>

using namespace std::chrono_literals;

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("node_pub1_2");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    auto mypub = node->create_publisher<std_msgs::msg::Int32>("topic_pub1_2", qos_profile);
    
    int count = 0;
    
    auto timer_callback = [node, mypub, &count]() {
        std_msgs::msg::Int32 message;
        message.data = count++;
        RCLCPP_INFO(node->get_logger(), "Publish: %d", message.data);
        mypub->publish(message);
    };
    
    auto timer = node->create_wall_timer(50ms, timer_callback);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}


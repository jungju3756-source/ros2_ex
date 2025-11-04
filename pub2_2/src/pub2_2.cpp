#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include <memory>
#include <chrono>
#include <functional>
using namespace std::chrono_literals;
void callback(rclcpp::Node::SharedPtr node,
rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr mypub,
const geometry_msgs::msg::Vector3 & message)
{
    RCLCPP_INFO(node->get_logger(), "Publish: %f, %f, %f", message.x, message.y, message.z);
    mypub->publish(message);
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("mynode");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    auto pub = node->create_publisher<geometry_msgs::msg::Vector3>("mytopic", qos_profile);

    // 입력은 한 번만 받고, 동일한 값을 주기적으로 무한 퍼블리시
    double x, y, z;
    std::cout << "보낼 Vector3 값 입력 (x y z): " << std::flush;
    while(!(std::cin >> x >> y >> z))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "잘못된 입력입니다. 다시 입력 (x y z): " << std::flush;
    }
    geometry_msgs::msg::Vector3 message;
    message.x = x;
    message.y = y;
    message.z = z;

    std::function<void()> fn = std::bind(callback, node, pub, message);
    auto timer = node->create_wall_timer(100ms, fn);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
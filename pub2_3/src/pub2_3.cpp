// Circle motion publisher for turtlesim using geometry_msgs::msg::Twist
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <chrono>
#include <functional>
#include <memory>

using namespace std::chrono_literals;

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("turtle_teleop_simple");


    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    auto publisher = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", qos_profile);

    // Expect 4 numbers: cmd.linear.x cmd.linear.y cmd.angular.x cmd.angular.y
    double lin_x = 0.5;
    double lin_y = 0.0;
    double ang_x = 0.0;
    double ang_y = 0.0;
    if (argc >= 5) {
        try {
            lin_x = std::stod(argv[1]);
            lin_y = std::stod(argv[2]);
            ang_x = std::stod(argv[3]);
            ang_y = std::stod(argv[4]);
        } catch (const std::exception & e) {
            RCLCPP_WARN(node->get_logger(), "Invalid args. Using defaults lin_x=0.5 lin_y=0 ang_x=0 ang_y=0");
        }
    } else {
        RCLCPP_INFO(node->get_logger(), "Usage: ros2 run pub2_3 pub2_3 <lin_x> <lin_y> <ang_x> <ang_y>  (publishing defaults if omitted)");
    }

    auto timer_callback = [node, publisher, lin_x, lin_y, ang_x, ang_y]() {
        geometry_msgs::msg::Twist cmd;
        cmd.linear.x = lin_x;
        cmd.linear.y = lin_y;
        cmd.linear.z = 0.0;
        cmd.angular.x = ang_x;
        cmd.angular.y = ang_y;
        cmd.angular.z = 0.0;

        publisher->publish(cmd);
        RCLCPP_DEBUG(node->get_logger(), "Publishing Twist: lin(%.2f, %.2f, 0), ang(%.2f, %.2f, 0)", lin_x, lin_y, ang_x, ang_y);
    };

    auto timer = node->create_wall_timer(100ms, timer_callback);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
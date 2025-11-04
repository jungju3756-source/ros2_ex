// teleop_turtle 대체: f,b,l,r 키 입력으로 /turtle1/cmd_vel 발행
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <iostream>
#include <limits>

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("node_pub1_3");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    auto publisher = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", qos_profile);

    const double linear_speed = 2.0;   // 전/후진 속도
    const double angular_speed = 2.0;  // 회전 속도

    RCLCPP_INFO(node->get_logger(), "키 입력: f(전진), b(후진), l(좌회전), r(우회전), q(종료)");
    char key;
    while (rclcpp::ok())
    {
        std::cout << "입력 > " << std::flush;
        if (!(std::cin >> key)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        geometry_msgs::msg::Twist cmd;
        switch (key)
        {
            case 'f': // 전진
                cmd.linear.x = linear_speed;
                cmd.angular.z = 0.0;
                break;
            case 'b': // 후진
                cmd.linear.x = -linear_speed;
                cmd.angular.z = 0.0;
                break;
            case 'l': // 좌회전
                cmd.linear.x = 0.0;
                cmd.angular.z = angular_speed;
                break;
            case 'r': // 우회전
                cmd.linear.x = 0.0;
                cmd.angular.z = -angular_speed;
                break;
            case 'q': // 종료
                rclcpp::shutdown();
                return 0;
            default:
                RCLCPP_WARN(node->get_logger(), "지원하지 않는 키: %c", key);
                continue;
        }

        publisher->publish(cmd);
        RCLCPP_INFO(node->get_logger(), "publish cmd_vel: linear.x=%.2f angular.z=%.2f", cmd.linear.x, cmd.angular.z);
    }

    rclcpp::shutdown();
    return 0;
}


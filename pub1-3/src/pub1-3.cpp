#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <iostream>

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("teleop_turtle_node");
  auto publisher = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);

  char c;
  while (rclcpp::ok()) {
    std::cout << "키 입력 (f-전진, b-후진, l-좌회전, r-우회전): ";
    std::cin >> c;

    geometry_msgs::msg::Twist msg;
    msg.linear.x = 0.0;
    msg.angular.z = 0.0;

    if (c == 'f') {
      msg.linear.x = 2.0;
    } else if (c == 'b') {
      msg.linear.x = -2.0;
    } else if (c == 'l') {
      msg.angular.z = 2.0;
    } else if (c == 'r') {
      msg.angular.z = -2.0;
    } else {
      std::cout << "잘못된 입력입니다. f, b, l, r 중 하나를 입력하세요.\n";
      continue;
    }

    publisher->publish(msg);
    std::cout << "명령 발행됨: " << c << std::endl;
  }

  rclcpp::shutdown();
  return 0;
}

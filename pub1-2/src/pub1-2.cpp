#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include <iostream>

class Pub1_2 : public rclcpp::Node {
public:
  Pub1_2() : Node("pub1_2"), count_(0) {
    publisher_ = this->create_publisher<geometry_msgs::msg::Vector3>("topic_pub1_2", 10);

    std::cout << "x, y, z 실수를 입력하세요: ";
    std::cin >> x_ >> y_ >> z_;

    timer_ = this->create_wall_timer(
      std::chrono::seconds(10),
      std::bind(&Pub1_2::timer_callback, this)
    );
  }

private:
  void timer_callback() {
    auto message = geometry_msgs::msg::Vector3();
    message.x = x_;
    message.y = y_;
    message.z = z_;
    publisher_->publish(message);

    RCLCPP_INFO(this->get_logger(), "Published count %d: x=%f, y=%f, z=%f", ++count_, x_, y_, z_);
  }

  rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  double x_, y_, z_;
  int count_;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Pub1_2>());
  rclcpp::shutdown();
  return 0;
}

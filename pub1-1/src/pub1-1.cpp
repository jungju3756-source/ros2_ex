// pub1_1.cpp
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

class Pub1_1 : public rclcpp::Node {
public:
  Pub1_1() : Node("pub1_1") {
    publisher_ = this->create_publisher<std_msgs::msg::Int32>("topic_pub1_1", 10);
    timer_ = this->create_wall_timer(
      std::chrono::seconds(1),
      std::bind(&Pub1_1::timer_callback, this)
    );
    count_ = 0;
  }

private:
  void timer_callback() {
    auto message = std_msgs::msg::Int32();
    message.data = count_++;
    publisher_->publish(message);
    RCLCPP_INFO(this->get_logger(), "Published: %d", message.data);
    if (count_ > 10) rclcpp::shutdown();  // 10회 발행 후 종료 (예시)
  }

  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  int count_;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Pub1_1>());
  rclcpp::shutdown();
  return 0;
}

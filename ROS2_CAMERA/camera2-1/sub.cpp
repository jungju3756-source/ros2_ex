#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include <opencv2/opencv.hpp>
#include <memory>
#include <iostream>

void image_callback(const sensor_msgs::msg::CompressedImage::SharedPtr msg, const std::string& window_name) {
    cv::Mat im = cv::imdecode(cv::Mat(msg->data), cv::IMREAD_COLOR);
    if (!im.empty()) {
        cv::imshow(window_name, im);
        cv::waitKey(1);
    }
}

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("camera2_sub");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort();

    auto sub_raw = node->create_subscription<sensor_msgs::msg::CompressedImage>(
        "image_raw", qos_profile,
        [node](const sensor_msgs::msg::CompressedImage::SharedPtr msg) { image_callback(msg, "image_raw"); });

    auto sub_gray = node->create_subscription<sensor_msgs::msg::CompressedImage>(
        "image_gray", qos_profile,
        [node](const sensor_msgs::msg::CompressedImage::SharedPtr msg) { image_callback(msg, "image_gray"); });

    auto sub_edge = node->create_subscription<sensor_msgs::msg::CompressedImage>(
        "image_edge", qos_profile,
        [node](const sensor_msgs::msg::CompressedImage::SharedPtr msg) { image_callback(msg, "image_edge"); });

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "opencv2/opencv.hpp"
#include <memory>
#include <functional>
#include <iostream>

using std::placeholders::_1;

void mysub_callback(rclcpp::Node::SharedPtr node,
                    const sensor_msgs::msg::CompressedImage::SharedPtr msg)
{
    // 압축된 이미지 → Mat으로 변환
    cv::Mat frame = cv::imdecode(cv::Mat(msg->data), cv::IMREAD_COLOR);

    if (frame.empty()) {
        RCLCPP_ERROR(node->get_logger(), "Empty frame received");
        return;
    }

    // 1) 원본 영상 출력
    cv::imshow("original", frame);

    // 2) 그레이스케일 변환
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::imshow("gray", gray);

    // 3) 이진영상 변환
    cv::Mat binary;
    cv::threshold(gray, binary, 100, 255, cv::THRESH_BINARY);
    cv::imshow("binary", binary);

    cv::waitKey(1);

    RCLCPP_INFO(node->get_logger(), 
        "Received Image : %s, %d x %d",
        msg->format.c_str(), frame.rows, frame.cols);
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("camsub_wsl6");

    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10)); // TCP
    // auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort(); // UDP

    std::function<void(const sensor_msgs::msg::CompressedImage::SharedPtr msg)> fn;
    fn = std::bind(mysub_callback, node, _1);

    auto mysub = node->create_subscription<sensor_msgs::msg::CompressedImage>(
        "image/compressed", qos_profile, fn);

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

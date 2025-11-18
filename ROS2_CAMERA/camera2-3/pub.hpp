#pragma once
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include <opencv2/opencv.hpp>

class CameraPublisher : public rclcpp::Node {
public:
    CameraPublisher();
    void publish_image();
private:
    rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr pub_;
    cv::VideoCapture cap_;
};

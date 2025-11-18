#pragma once
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include <opencv2/opencv.hpp>
#include <string>

class CameraSubscriber : public rclcpp::Node {
public:
    CameraSubscriber();
    ~CameraSubscriber();
private:
    rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr sub_;
    cv::VideoWriter writer_;
    void callback(sensor_msgs::msg::CompressedImage::UniquePtr msg);
};

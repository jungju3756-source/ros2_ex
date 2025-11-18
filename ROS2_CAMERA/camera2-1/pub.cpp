#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <chrono>

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("camera2_pub");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort(); // UDP

    auto pub_raw = node->create_publisher<sensor_msgs::msg::CompressedImage>("image_raw", qos_profile);
    auto pub_gray = node->create_publisher<sensor_msgs::msg::CompressedImage>("image_gray", qos_profile);
    auto pub_edge = node->create_publisher<sensor_msgs::msg::CompressedImage>("image_edge", qos_profile);

    std::string src = "nvarguscamerasrc sensor-id=0 ! ... ! appsink";  // Jetson Camera pipeline
    cv::VideoCapture cap(src, cv::CAP_GSTREAMER);
    if (!cap.isOpened()) { RCLCPP_ERROR(node->get_logger(), "Could not open camera!"); return -1; }

    cv::Mat frame;
    rclcpp::WallRate loop_rate(30.0); // 30Hz
    while (rclcpp::ok()) {
        cap >> frame;
        if (frame.empty()) break;

        cv::Mat gray, edge;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, edge, 100, 200);

        // raw
        auto msg_raw = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toCompressedImageMsg();
        pub_raw->publish(*msg_raw);

        // gray
        auto msg_gray = cv_bridge::CvImage(std_msgs::msg::Header(), "mono8", gray).toCompressedImageMsg();
        pub_gray->publish(*msg_gray);

        // edge
        auto msg_edge = cv_bridge::CvImage(std_msgs::msg::Header(), "mono8", edge).toCompressedImageMsg();
        pub_edge->publish(*msg_edge);

        loop_rate.sleep();
    }
    rclcpp::shutdown();
    return 0;
}

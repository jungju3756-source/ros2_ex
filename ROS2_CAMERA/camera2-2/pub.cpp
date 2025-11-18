#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("camera2_pub");
    auto pub = node->create_publisher<sensor_msgs::msg::CompressedImage>("image_compressed", 10);

    std::string src = "nvarguscamerasrc sensor-id=0 ! ... ! appsink";
    cv::VideoCapture cap(src, cv::CAP_GSTREAMER);
    if (!cap.isOpened()) { RCLCPP_ERROR(node->get_logger(), "Could not open camera!"); return -1; }

    cv::Mat frame;
    rclcpp::WallRate loop_rate(30.0); // 30Hz
    std_msgs::msg::Header hdr;
    while (rclcpp::ok()) {
        cap >> frame;
        if (frame.empty()) break;
        auto msg = cv_bridge::CvImage(hdr, "bgr8", frame).toCompressedImageMsg();
        pub->publish(*msg);
        loop_rate.sleep();
    }
    rclcpp::shutdown();
    return 0;
}

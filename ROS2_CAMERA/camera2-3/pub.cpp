#include "pub.hpp"
#include <cv_bridge/cv_bridge.h>

CameraPublisher::CameraPublisher()
    : Node("camera2_pub")
{
    pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>("image_compressed", 10);
    cap_.open("nvarguscamerasrc sensor-id=0 ! ... ! appsink", cv::CAP_GSTREAMER);
}

void CameraPublisher::publish_image() {
    cv::Mat frame;
    rclcpp::WallRate loop_rate(30.0);
    std_msgs::msg::Header hdr;
    while (rclcpp::ok()) {
        cap_ >> frame;
        if (frame.empty()) break;
        auto msg = cv_bridge::CvImage(hdr, "bgr8", frame).toCompressedImageMsg();
        pub_->publish(*msg);
        loop_rate.sleep();
    }
}

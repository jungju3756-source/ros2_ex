#include "sub.hpp"

CameraSubscriber::CameraSubscriber()
    : Node("camera2_sub")
{
    sub_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(
        "image_compressed", 10,
        std::bind(&CameraSubscriber::callback, this, std::placeholders::_1));

    writer_.open("output.mp4", cv::VideoWriter::fourcc('m','p','4','v'), 30, cv::Size(640, 360));
}

CameraSubscriber::~CameraSubscriber() { writer_.release(); }

void CameraSubscriber::callback(sensor_msgs::msg::CompressedImage::UniquePtr msg) {
    cv::Mat frame = cv::imdecode(cv::Mat(msg->data), cv::IMREAD_COLOR);
    if (!frame.empty()) {
        cv::imshow("화면영상", frame);
        cv::waitKey(1);
        writer_.write(frame);
    }
}

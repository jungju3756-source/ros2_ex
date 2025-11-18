#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>

class VideoSaver {
public:
    VideoSaver(const std::string &filename, int width, int height, int fps)
        : filename_(filename), writer_(filename, cv::VideoWriter::fourcc('m','p','4','v'), fps, cv::Size(width, height)) {}
    void write(const cv::Mat &frame) { writer_.write(frame); }
    void release() { writer_.release(); }
private:
    std::string filename_;
    cv::VideoWriter writer_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("camera2_sub");
    auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort();

    VideoSaver saver("output.mp4", 640, 360, 30); // 저장할 파일명, 해상도, FPS
    auto callback = [&saver](sensor_msgs::msg::CompressedImage::SharedPtr msg) {
        cv::Mat frame = cv::imdecode(cv::Mat(msg->data), cv::IMREAD_COLOR);
        if (!frame.empty()) {
            cv::imshow("frame", frame);
            cv::waitKey(1);
            saver.write(frame);
        }
    };

    auto sub = node->create_subscription<sensor_msgs::msg::CompressedImage>(
        "image_compressed", qos, callback);

    rclcpp::spin(node);
    saver.release();
    rclcpp::shutdown();
    return 0;
}

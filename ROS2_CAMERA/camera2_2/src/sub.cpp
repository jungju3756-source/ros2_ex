#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "opencv2/opencv.hpp"
#include <memory>
#include <functional>
#include <iostream>
using std::placeholders::_1;

cv::VideoWriter writer;
bool writer_initialized = false;

void mysub_callback(rclcpp::Node::SharedPtr node, const sensor_msgs::msg::CompressedImage::SharedPtr msg)
{
    cv::Mat frame = cv::imdecode(cv::Mat(msg->data), cv::IMREAD_COLOR);

    if(frame.empty()){
        return;
    }

    cv::imshow("wsl", frame);
    cv::waitKey(1);

    if(!writer_initialized){
        int width = frame.cols;
        int height = frame.rows;
        int fps = 30;

        writer.open("output.mp4",
                    cv::VideoWriter::fourcc('m','p','4','v'),
                    fps,
                    cv::Size(width, height));

        if(writer.isOpened()){
            writer_initialized = true;
        }
    }

    if(writer_initialized){
        writer.write(frame);
    }

    RCLCPP_INFO(node->get_logger(), "Received Image : %s,%d,%d",
                msg->format.c_str(), frame.rows, frame.cols);
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("camsub_wsl_save");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));

    std::function<void(const sensor_msgs::msg::CompressedImage::SharedPtr msg)> fn;
    fn = std::bind(mysub_callback, node, _1);

    auto mysub = node->create_subscription<sensor_msgs::msg::CompressedImage>(
        "image/compressed", qos_profile, fn);

    rclcpp::spin(node);

    if(writer_initialized){
        writer.release();
        std::cout << "Video saved successfully." << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}

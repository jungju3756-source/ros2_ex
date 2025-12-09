#ifndef _PUB_HPP_
#define _PUB_HPP_

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "opencv2/opencv.hpp"
#include <memory>
#include <vector>

// CamPublisher 클래스: 비디오 파일을 읽어 ROS2 이미지 메시지로 발행
class CamPublisher : public rclcpp::Node
{
public:
    // 생성자: 비디오 경로를 입력받아 초기화
    explicit CamPublisher(const std::string &video_path);
    // 비디오 재생 및 발행 루프 함수
    void run();

private:
    std::string video_path_;       // 비디오 파일 경로
    cv::VideoCapture cap_;         // OpenCV 비디오 캡처 객체
    rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr pub_; // 이미지 발행자
    rclcpp::WallRate loop_rate_;   // 루프 주기 (30Hz)
};

#endif

#include "linedetect_wsl_2/pub.hpp"

// 생성자
CamPublisher::CamPublisher(const std::string &video_path)
: Node("campub_video"),
  video_path_(video_path),
  loop_rate_(30) // 30Hz로 발행 속도 설정
{
    // "image/compressed" 토픽으로 발행할 퍼블리셔 생성 (테라코타 QoS: KeepLast 10)
    pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>(
        "image/compressed", 10);
}

// 메인 실행 함수
void CamPublisher::run()
{
    // 비디오 파일 열기
    cap_.open(video_path_);
    if (!cap_.isOpened()) {
        RCLCPP_ERROR(this->get_logger(), "Could not open video!");
        return;
    }

    cv::Mat frame;
    // ROS가 실행되는 동안 계속 반복
    while (rclcpp::ok()) {
        cap_ >> frame;
        if (frame.empty()) {
            // 비디오가 끝나면 처음으로 되감기 (무한 반복)
            cap_.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }

        // OpenCV Mat 이미지를 ROS 압축 이미지 메시지로 변환
        sensor_msgs::msg::CompressedImage msg;
        cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toCompressedImageMsg(msg);
        
        // 메시지 발행
        pub_->publish(msg);
        
        // 콘솔에 로그 출력을 줄이기 위해 주석 처리됨
        // RCLCPP_INFO(this->get_logger(), "Published frame");
        
        // 이벤트를 처리하고 주기(30Hz)를 맞춤
        rclcpp::spin_some(this->get_node_base_interface());
        loop_rate_.sleep();
    }
}

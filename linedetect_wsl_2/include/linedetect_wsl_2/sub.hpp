#ifndef _SUB_HPP_2_
#define _SUB_HPP_2_
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "opencv2/opencv.hpp"
#include <memory>
#include <chrono>

using namespace std;
using namespace cv;

// LineSubscriber 클래스: ROS2 노드로서 이미지를 구독하고 라인을 감지합니다.
class LineSubscriber : public rclcpp::Node
{
    public:
        LineSubscriber();
    private:
    // 이미지 콜백 함수: 압축 이미지를 수신할 때마다 호출됩니다.
    void callback(const sensor_msgs::msg::CompressedImage::SharedPtr msg);
    
    // 이미지 구독자
    rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr sub_;
    
    // Lane 구조체: 개별 차선의 상태를 관리합니다.
    struct Lane {
        cv::Rect rect;       // 바운딩 박스 (위치 및 크기)
        cv::Point centroid;  // 중심점 좌표
        int missing_frames;  // 라인이 감지되지 않은 프레임 수 (잔상 유지용)
        bool active;         // 현재 추적 중인지 여부
    };

    // 차선 상태를 업데이트하는 헬퍼 함수
    void process_lane(Lane& lane, const cv::Rect& new_rect, const cv::Point& new_centroid, bool detected);
    
    double prev_x_; // (사용되지 않음, 호환성 유지)
    
    // 차선 배열: 0번은 왼쪽(Red), 1번은 오른쪽(Blue)
    Lane lanes_[2]; 
    
    // 최대 잔상 유지 프레임 수 (30프레임 동안 라인이 안 보여도 위치 기억)
    const int MAX_MISSING_FRAMES = 30;
    
    // 급격한 위치 변화를 무시하기 위한 임계값 (픽셀 단위)
    const double MAX_MOVE_THRESHOLD = 80.0;
};
#endif
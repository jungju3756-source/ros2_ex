#include "linedetect_wsl_2/pub.hpp"

int main(int argc, char * argv[]){
    // ROS2 노드 초기화
    rclcpp::init(argc,argv);
    
    // 비디오 파일 경로 설정 (시뮬레이션 용도)
   
    //std::string video_path = "/home/linux/simulation/7_lt_ccw_100rpm_in.mp4";
    std::string video_path = "/home/linux/simulation/5_lt_cw_100rpm_out.mp4";
    
    // CamPublisher 노드 실행
    auto node = std::make_shared<CamPublisher>(video_path);
    node->run();
    
    // 종료
    rclcpp::shutdown();
    return 0;
}
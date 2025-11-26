#include "rclcpp/rclcpp.hpp"
#include "dxl3/dxl.hpp"
#include <memory>
#include <chrono>
#include <iostream>

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("dxl3");

    rclcpp::WallRate loop_rate(10.0);   // 10Hz → 0.1초

    Dxl mx;
    if(!mx.open())
    {
        RCLCPP_INFO(node->get_logger(), "dynamixel open error\n");
        rclcpp::shutdown();
        return -1;
    }

    int vel = 0;
    int direction = 1;   // 1 증가 / -1 감소

    while(rclcpp::ok())
    {
        // 속도 업데이트
        vel += direction * 10;

        // 경계값에서 방향 반전
        if(vel >= 100) direction = -1;
        if(vel <= -100) direction = 1;

        // 좌우 동일 속도 적용
        mx.setVelocity(vel, vel);

        RCLCPP_INFO(node->get_logger(),
                    "left speed:%d, right speed:%d", vel, vel);

        loop_rate.sleep();
    }

    mx.close();
    rclcpp::shutdown();
    return 0;
}

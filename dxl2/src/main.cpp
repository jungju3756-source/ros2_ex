#include "rclcpp/rclcpp.hpp"
#include "dxl2/dxl.hpp"
#include <memory>
#include <chrono>
#include <iostream>
int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("dxl2");
    rclcpp::WallRate loop_rate(20.0);
    Dxl mx;
    int vel1 = 0,vel2 = 0;
    if(!mx.open())
    {
    RCLCPP_INFO(node->get_logger(), "dynamixel open error\n");
    rclcpp::shutdown();
    return -1; //장치열기
    }
    while(rclcpp::ok())
    {
        if (mx.kbhit()) //키보드입력 체크
        {
            char c = mx.getch(); //키입력 받기
            switch(c)
            {
                case 's': vel1 = 0; vel2 = 0; break; //정지
                case 'f': vel1 = 50; vel2 = -50; break; //전진
                case 'b': vel1 = -50; vel2 = 50; break; //후진
                case 'l': vel1 = -50; vel2 = -50; break; //좌회전
                case 'r': vel1 = 50; vel2 = 50; break; //우회전
                default : vel1 = 0; vel2 = 0; break; //정지
            }
            mx.setVelocity(vel1,vel2);
        }
        RCLCPP_INFO(node->get_logger(),"left speed:%d,right speed:%d",vel1,vel2);

        loop_rate.sleep();
    }
    mx.close();
    rclcpp::shutdown();
    return 0;
}
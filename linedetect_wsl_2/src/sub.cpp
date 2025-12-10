#include "linedetect_wsl_2/sub.hpp"
using namespace cv;
using namespace std;
using std::placeholders::_1;

// LineSubscriber 생성자 (유지)
LineSubscriber::LineSubscriber(): Node("camsub_wsl"), prev_x_(-1.0)
{
    // 차선 추적기 초기화
    for(int i=0; i<2; i++) {
        lanes_[i].active = false;
        lanes_[i].missing_frames = 0;
        lanes_[i].rect = cv::Rect(0,0,0,0);
        lanes_[i].centroid = cv::Point(0,0);
    }
    
    // 구독 설정
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    sub_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(
        "image/compressed",
        qos_profile,
        std::bind(&LineSubscriber::callback, this, _1)
    );
    RCLCPP_INFO(this->get_logger(), "LineSubscriber Node Started");
}

// 헬퍼 함수: 차선 상태 갱신 (유지)
void LineSubscriber::process_lane(Lane& lane, const cv::Rect& new_rect, const cv::Point& new_centroid, bool detected) {
    if (detected) {
        if (!lane.active || lane.missing_frames > MAX_MISSING_FRAMES) {
            lane.rect = new_rect;
            lane.centroid = new_centroid;
            lane.active = true;
        } else {
            double alpha = 0.4; // EMA Smoothing
            lane.rect.x = (1.0 - alpha) * lane.rect.x + alpha * new_rect.x;
            lane.rect.y = (1.0 - alpha) * lane.rect.y + alpha * new_rect.y;
            lane.rect.width = (1.0 - alpha) * lane.rect.width + alpha * new_rect.width;
            lane.rect.height = (1.0 - alpha) * lane.rect.height + alpha * new_rect.height;
            
            lane.centroid.x = (1.0 - alpha) * lane.centroid.x + alpha * new_centroid.x;
            lane.centroid.y = (1.0 - alpha) * lane.centroid.y + alpha * new_centroid.y;
        }
        lane.missing_frames = 0;
    } else if (lane.active) {
        lane.missing_frames++;
        if (lane.missing_frames > MAX_MISSING_FRAMES) {
            lane.active = false;
        }
    }
}

// 콜백 함수 (수정됨)
void LineSubscriber::callback(
        const sensor_msgs::msg::CompressedImage::SharedPtr msg)
{
    auto startTime = chrono::steady_clock::now();

    Mat frame, gray, binary, labels, stats, centroids, corrected_gray;
    double error = 0.0;
    
    // [이미지 전처리]
    frame = cv::imdecode(Mat(msg->data), IMREAD_COLOR);
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    Scalar avg = mean(gray);
    corrected_gray = gray + (160.0 - avg.val[0]);
    threshold(corrected_gray, binary, 210, 255, THRESH_BINARY);

    int h = binary.rows;
    int w = binary.cols;
    Rect roi(0, 3 * h / 4, w, h / 4);
    Mat binary_half = binary(roi).clone();

    // [Blob 분석]
    int cnt = connectedComponentsWithStats(binary_half, labels, stats, centroids);
    double mid_x = binary_half.cols / 2.0;

    int best_idx_0 = -1; // 라인 0: 메인 라인 (빨강)
    double min_dist_0 = DBL_MAX;
    
    int best_idx_1 = -1; // 라인 1: 커브 라인 (파랑) - 추적용으로 하나만 선택
    double min_dist_1 = DBL_MAX;
    
    // 예상 위치
    double target_x_0 = lanes_[0].active ? lanes_[0].centroid.x : mid_x;
    double target_x_1 = lanes_[1].active ? lanes_[1].centroid.x : mid_x;
    
    // ***************************************************************
    // ** 1단계: 메인 라인 (라인 0, 빨강) 최적 후보 검색 **
    // ***************************************************************
    for (int i = 1; i < cnt; i++)
    {
        int* p = stats.ptr<int>(i);
        if (p[4] < 200 || p[4] > 7000) continue; 
        double cx = centroids.at<double>(i, 0);

        // 라인 0과의 거리 측정
        double dist_0 = abs(target_x_0 - cx);
        if (!(lanes_[0].active && dist_0 > MAX_MOVE_THRESHOLD) && dist_0 < min_dist_0) {
             min_dist_0 = dist_0;
             best_idx_0 = i;
        }
    }

    // ***************************************************************
    // ** 2단계: 커브 라인 (라인 1, 파랑) 최적 후보 검색 (추적용) **
    // ***************************************************************
    for (int i = 1; i < cnt; i++)
    {
        if (i == best_idx_0) continue; // 메인 라인 제외

        int* p = stats.ptr<int>(i);
        if (p[4] < 200 || p[4] > 7000) continue; 
        double cx = centroids.at<double>(i, 0);

        // 라인 1과의 거리 측정
        double dist_1 = abs(target_x_1 - cx);
        if (!(lanes_[1].active && dist_1 > MAX_MOVE_THRESHOLD) && dist_1 < min_dist_1) {
             min_dist_1 = dist_1;
             best_idx_1 = i; // 추적에 사용될 하나의 커브 라인만 선택
        }
    }
    
    // [초기 지정 로직]
    if (!lanes_[0].active && !lanes_[1].active) {
        if (best_idx_0 == -1 && best_idx_1 != -1) {
            // 커브 후보만 찾은 경우, 커브를 메인으로 승격
            best_idx_0 = best_idx_1;
            best_idx_1 = -1;
        }
    }

    // [라인 상태 업데이트]
    // 라인 0 (빨강/메인) 상태 업데이트
    bool lane0_detected;
    {
        lane0_detected = (best_idx_0 != -1);
        cv::Rect new_rect;
        cv::Point new_centroid;
        if (lane0_detected) {
            int* q = stats.ptr<int>(best_idx_0);
            new_rect = cv::Rect(q[0], q[1], q[2], q[3]);
            new_centroid = cv::Point(centroids.at<double>(best_idx_0, 0), centroids.at<double>(best_idx_0, 1));
        }
        process_lane(lanes_[0], new_rect, new_centroid, lane0_detected);
    }
    
    // 라인 1 (파랑/커브) 상태 업데이트
    bool lane1_detected;
    {
        lane1_detected = (best_idx_1 != -1);
        cv::Rect new_rect;
        cv::Point new_centroid;
        if (lane1_detected) {
            int* q = stats.ptr<int>(best_idx_1);
            new_rect = cv::Rect(q[0], q[1], q[2], q[3]);
            new_centroid = cv::Point(centroids.at<double>(best_idx_1, 0), centroids.at<double>(best_idx_1, 1));
        }
        process_lane(lanes_[1], new_rect, new_centroid, lane1_detected);
    }
    
    // [시각화 로직 수정: 메인을 제외한 모든 유효 Blob을 파랑으로, 잔상 포함]
    cvtColor(binary_half, binary_half, COLOR_GRAY2BGR);
    
    // 1. 현재 프레임에서 감지된 Blob 시각화 (빨강 vs 파랑)
    for (int i = 1; i < cnt; i++)
    {
        int* p = stats.ptr<int>(i);
        // 유효 Blob인지 확인
        if (p[4] < 200 || p[4] > 7000) continue;
        double cx = centroids.at<double>(i, 0);
        
        // 메인 라인 (빨강)
        if (i == best_idx_0) {
            circle(binary_half, Point(cx, centroids.at<double>(i, 1)), 3, Scalar(0,0,255), -1); 
            rectangle(binary_half, Rect(p[0], p[1], p[2], p[3]), Scalar(0,0,255), 2);
        } 
        // 메인 라인이 아닌 모든 유효 Blob (파랑)
        else {
            circle(binary_half, Point(cx, centroids.at<double>(i, 1)), 3, Scalar(255,0,0), -1); 
            rectangle(binary_half, Rect(p[0], p[1], p[2], p[3]), Scalar(255,0,0), 2);
        }
    }
    
    // 2. 라인이 끊겼을 때 잔상 시각화 (색상 유지)
    //빨강 라인 잔상
    if (!lane0_detected && lanes_[0].active) {
        // 끊겼지만 추적을 유지 중이라면 (active) 잔상(이전 위치)을 빨강으로 표시
        circle(binary_half, lanes_[0].centroid, 3, Scalar(0,0,255), -1); 
    }
   //파랑 라인 잔상
    if (!lane1_detected && lanes_[1].active) {
        // 끊겼지만 추적을 유지 중이라면 (active) 잔상(이전 위치)을 파랑으로 표시
        circle(binary_half, lanes_[1].centroid, 3, Scalar(255,0,0), -1); 
    }


    // [에러 (중심) 계산]
    double detected_center = mid_x;
    
    // 에러 계산은 추적되는 lanes_[0] (빨강)와 lanes_[1] (파랑) 두 개를 기반으로 함 (유지)
    if (lanes_[0].active && lanes_[1].active) {
        detected_center = (lanes_[0].centroid.x + lanes_[1].centroid.x) / 2.0;
    } else if (lanes_[0].active) {
        detected_center = lanes_[0].centroid.x < mid_x ? lanes_[0].centroid.x + 160 : lanes_[0].centroid.x - 160; 
    } else if (lanes_[1].active) {
        detected_center = lanes_[1].centroid.x < mid_x ? lanes_[1].centroid.x + 160 : lanes_[1].centroid.x - 160; 
    }
    
    error = mid_x - detected_center;
    
    // [결과 출력]
    imshow("Original", frame);
    imshow("Binary ROI Result", binary_half);
    waitKey(1);

    auto endTime = chrono::steady_clock::now();
    float totalTime = chrono::duration<double, milli>(endTime - startTime).count();

    cout << "err:" << (int)error 
         << ", detected_center_x:" << (int)detected_center
         << ", time:" << totalTime << endl;
}

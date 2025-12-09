#include "linedetect_wsl_2/sub.hpp"
using namespace cv;
using namespace std;
using std::placeholders::_1;

// 생성자: 노드를 초기화하고 구독자를 설정합니다.
LineSubscriber::LineSubscriber(): Node("camsub_wsl"), prev_x_(-1.0)
{
    // 차선 추적기 초기화 (비활성화 상태로 시작)
    for(int i=0; i<2; i++) {
        lanes_[i].active = false;
        lanes_[i].missing_frames = 0;
        lanes_[i].rect = cv::Rect(0,0,0,0);
    }
    
    // QoS 설정: 최근 10개의 메시지만 유지
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));

    // "image/compressed" 토픽 구독 설정
    sub_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(
        "image/compressed",
        qos_profile,
        std::bind(&LineSubscriber::callback, this, std::placeholders::_1)
    );

    RCLCPP_INFO(this->get_logger(), "LineSubscriber Node Started");
}

// 콜백 함수: 이미지를 수신하고 처리하는 메인 로직입니다.
void LineSubscriber::callback(
        const sensor_msgs::msg::CompressedImage::SharedPtr msg)
{
    auto startTime = chrono::steady_clock::now();

    Mat frame, gray, binary, labels, stats, centroids, corrected_gray;
    double current_mean, target_mean, diff, error = 0.0;
    int h, w, cnt;
    
    // 이미지를 디코딩하여 OpenCV Mat 형식으로 변환 (BGR)
    frame = cv::imdecode(Mat(msg->data), IMREAD_COLOR);
    // 흑백(Grayscale) 변환
    cvtColor(frame, gray, COLOR_BGR2GRAY);

    // 밝기 보정: 평균 밝기를 160으로 맞춤
    Scalar avg = mean(gray);
    current_mean = avg.val[0];
    target_mean = 160.0;
    diff = target_mean - current_mean;
    corrected_gray = gray + diff;

    // 이진화 (Thresholding): 밝은 부분(라인)만 흰색으로 추출
    threshold(corrected_gray, binary, 210, 255, THRESH_BINARY);

    h = binary.rows;
    w = binary.cols;
    
    // 관심 영역(ROI) 설정: 이미지의 하단 1/4만 사용
    Rect roi(0, 3 * h / 4, w, h / 4);
    Mat binary_half = binary(roi).clone();

    // 레이블링: 연결된 픽셀 덩어리(Blob)를 찾음
    cnt = connectedComponentsWithStats(binary_half, labels, stats, centroids);
    
    // 화면을 좌우로 나누어 추적할 기준점 설정
    double mid_x = binary_half.cols / 2.0;

    int best_idx_left = -1;
    double min_dist_left = DBL_MAX;
    
    int best_idx_right = -1;
    double min_dist_right = DBL_MAX;
    
    // 예상 위치 (Persistence): 라인이 사라지면 마지막 위치를 기준으로 찾음
    // 0번: 왼쪽(Red), 1번: 오른쪽(Blue)
    double target_x_left = lanes_[0].active ? lanes_[0].centroid.x : mid_x * 0.5;
    double target_x_right = lanes_[1].active ? lanes_[1].centroid.x : mid_x * 1.5;

    // 감지된 각 Blob(덩어리)을 순회하며 분석
    for (int i = 1; i < cnt; i++)
    {
        int* p = stats.ptr<int>(i);
        // 노이즈 제거: 면적이 너무 작거나 큰 Blob은 무시 (200 ~ 7000 픽셀)
        if (p[4] < 200 || p[4] > 7000) continue;

        double cx = centroids.at<double>(i, 0);

        // Blob을 위치에 따라 왼쪽 또는 오른쪽 후보로 분류
        if (cx < mid_x) { // 왼쪽 영역 후보
             double dist = abs(target_x_left - cx);
             
             // 이상치 제거 (Outlier Rejection): 갑자기 80픽셀 이상 튄 경우 무시
             if (lanes_[0].active && dist > MAX_MOVE_THRESHOLD) continue;
             
             // 가장 가까운 후보 선택
             if (dist < min_dist_left) {
                 min_dist_left = dist;
                 best_idx_left = i;
             }
        } else { // 오른쪽 영역 후보
             double dist = abs(target_x_right - cx);
             
             // 이상치 제거
             if (lanes_[1].active && dist > MAX_MOVE_THRESHOLD) continue;
             
             if (dist < min_dist_right) {
                 min_dist_right = dist;
                 best_idx_right = i;
             }
        }
    }

    // 결과 시각화를 위해 BGR로 변환
    cvtColor(binary_half, binary_half, COLOR_GRAY2BGR);
    
    // 시각화 루프: 모든 유효한 후보를 점(Dot)으로 표시
    for (int i = 1; i < cnt; i++)
    {
        int* p = stats.ptr<int>(i);
        if (p[4] < 200 || p[4] > 7000) continue;
        
        double cx = centroids.at<double>(i, 0);
        double cy = centroids.at<double>(i, 1);
        
        // 왼쪽 영역은 빨간 점, 오른쪽 영역은 파란 점
        Scalar color = (cx < mid_x) ? Scalar(0,0,255) : Scalar(255,0,0);
        
        // 점 그리기
        circle(binary_half, Point(cx, cy), 3, color, -1);
        
        // 현재 추적 중인 베스트 라인이면 바운딩 박스를 그 덩어리 위에 그리기
        // (라인이 사라졌을 때는 여기서 그려지지 않음)
        if (i == best_idx_left) {
            rectangle(binary_half, Rect(p[0], p[1], p[2], p[3]), Scalar(0,0,255), 2);
        }
        
        if (i == best_idx_right) {
            rectangle(binary_half, Rect(p[0], p[1], p[2], p[3]), Scalar(255,0,0), 2);
        }
    }
    
    // 왼쪽 라인 상태 업데이트 (Index 0, Red)
    {
        bool detected = (best_idx_left != -1);
        cv::Rect new_rect;
        cv::Point new_centroid;
        if (detected) {
            int* q = stats.ptr<int>(best_idx_left);
            new_rect = cv::Rect(q[0], q[1], q[2], q[3]);
            new_centroid = cv::Point(centroids.at<double>(best_idx_left, 0), centroids.at<double>(best_idx_left, 1));
        }
        // 헬퍼 함수를 통해 라인 상태(active, missing_frames, smoothing) 업데이트
        process_lane(lanes_[0], new_rect, new_centroid, detected);
        
        // 라인을 놓쳤지만(active), 아직 잔상 유지 기간인 경우 점만 표시
        if (!detected && lanes_[0].active) {
             circle(binary_half, lanes_[0].centroid, 3, Scalar(0,0,255), -1);
        }
    }
    
    // 오른쪽 라인 상태 업데이트 (Index 1, Blue)
    {
        bool detected = (best_idx_right != -1);
        cv::Rect new_rect;
        cv::Point new_centroid;
        if (detected) {
            int* q = stats.ptr<int>(best_idx_right);
            new_rect = cv::Rect(q[0], q[1], q[2], q[3]);
            new_centroid = cv::Point(centroids.at<double>(best_idx_right, 0), centroids.at<double>(best_idx_right, 1));
        }
        process_lane(lanes_[1], new_rect, new_centroid, detected);
        
        // 잔상 시각화
        if (!detected && lanes_[1].active) {
             circle(binary_half, lanes_[1].centroid, 3, Scalar(255,0,0), -1);
        }
    }

    // 에러(Error) 계산: 이미지 중심과 감지된 라인 중심의 차이
    double detected_center = mid_x;
    if (lanes_[0].active && lanes_[1].active) {
        // 양쪽 라인이 다 보이면 두 라인의 중간을 사용
        detected_center = (lanes_[0].centroid.x + lanes_[1].centroid.x) / 2.0;
    } else if (lanes_[0].active) {
        // 왼쪽만 보이면 왼쪽 라인에서 160(절반 폭 가정)만큼 오른쪽으로
        detected_center = lanes_[0].centroid.x + 160; 
    } else if (lanes_[1].active) {
        // 오른쪽만 보이면 오른쪽 라인에서 160만큼 왼쪽으로
        detected_center = lanes_[1].centroid.x - 160; 
    } else {
        // 라인이 안 보이면 중심 유지 (또는 이전 값을 사용할 수도 있음)
        detected_center = mid_x;
    }

    error = mid_x - detected_center;
    
    // 에러에 따른 좌/우 모터 속도 계산 (P 제어와 유사)
    int lvel = 150 - (int)error; 
    int rvel = 150 + (int)error;

    // 결과 이미지 출력
    imshow("Original", frame);
    imshow("Binary ROI Result", binary_half);
    waitKey(1);

    auto endTime = chrono::steady_clock::now();
    float totalTime = chrono::duration<double, milli>(endTime - startTime).count();

    // 터미널 출력: 에러, 좌우 속도, 처리 시간
    cout << "err:" << (int)error 
         << ", lvel:" << lvel 
         << ", rvel:" << rvel 
         << ", time:" << totalTime << endl;
}

// 헬퍼 함수: 개별 라인의 추적 상태를 관리하고 스무딩(EMA)을 적용합니다.
void LineSubscriber::process_lane(Lane& lane, const cv::Rect& new_rect, const cv::Point& new_centroid, bool detected) {
    if (detected) {
        // 처음 감지되었거나, 오랫동안 놓쳤다가 다시 찾은 경우 -> 바로 위치 갱신
        if (!lane.active || lane.missing_frames > MAX_MISSING_FRAMES) {
            lane.rect = new_rect;
            lane.centroid = new_centroid;
            lane.active = true;
            lane.missing_frames = 0;
        } else {
            // 이미 추적 중인 경우 -> EMA(지수 이동 평균)로 부드럽게 갱신
            // alpha 값이 작을수록 더 부드럽지만 반응이 느려짐 (현재 0.4)
            double alpha = 0.4;
            lane.rect.x = (1.0 - alpha) * lane.rect.x + alpha * new_rect.x;
            lane.rect.y = (1.0 - alpha) * lane.rect.y + alpha * new_rect.y;
            lane.rect.width = (1.0 - alpha) * lane.rect.width + alpha * new_rect.width;
            lane.rect.height = (1.0 - alpha) * lane.rect.height + alpha * new_rect.height;
            
            lane.centroid.x = (1.0 - alpha) * lane.centroid.x + alpha * new_centroid.x;
            lane.centroid.y = (1.0 - alpha) * lane.centroid.y + alpha * new_centroid.y;
            
            lane.missing_frames = 0;
        }
    } else {
        // 라인을 놓친 경우
        if (lane.active) {
            lane.missing_frames++;
            // 일정 시간(MAX_MISSING_FRAMES) 이상 놓치면 추적 중지
            if (lane.missing_frames > MAX_MISSING_FRAMES) {
                lane.active = false;
            }
        }
    }
}
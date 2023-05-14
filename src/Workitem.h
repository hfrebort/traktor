#pragma once

struct Workitem {
    cv::Mat frame;
    std::atomic<bool> isValidForAnalyse;
};
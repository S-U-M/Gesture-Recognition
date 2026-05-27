#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "SkinSegmentation.h"
#include "HandAnalysis.h"
#include "GestureClassifier.h"
#include "Visualization.h"

static void processFrame(const cv::Mat& frame, bool showMask = false)
{
    // Keep full resolution. The previous 0.35 resize made fixed area/depth
    // thresholds reject real hand contours and finger gaps.
    cv::Mat display = frame.clone();

    cv::Mat mask = segmentSkin(frame);
    HandFeatures features = analyzeHand(mask);
    std::string gesture = classifyGesture(features);

    drawResults(display, features, gesture);

    cv::imshow("Gesture Recognition", display);

    if (showMask) {
        cv::imshow("Skin Mask (debug)", mask);
    }

    std::cout << "[Result] " << gesture
        << " | fingers=" << features.fingerCount
        << " solidity=" << features.solidity
        << " extent=" << features.areaRatio
        << " AR=" << features.aspectRatio
        << "\n";
}

int main(int argc, char* argv[])
{
    std::cout << "Hand Gesture Recognition\n";
    std::cout << "========================\n";

    std::vector<std::string> paths;
    for (int i = 1; i < argc; ++i) {
        paths.emplace_back(argv[i]);
    }

    if (paths.empty()) {
        paths.emplace_back("thumbs_up.jpg");
    }

    constexpr bool SHOW_MASK = true;

    for (const auto& path : paths) {
        cv::Mat frame = cv::imread(path);
        if (frame.empty()) {
            std::cerr << "[Error] Could not load: " << path << "\n";
            continue;
        }

        std::cout << "\n[Image] " << path << "\n";
        processFrame(frame, SHOW_MASK);

        std::cout << "Press any key for next image...\n";
        cv::waitKey(0);
    }

    cv::destroyAllWindows();
    return 0;
}

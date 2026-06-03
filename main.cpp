#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "Constants.h"
#include "SkinSegmentation.h"
#include "HandAnalysis.h"
#include "GestureClassifier.h"
#include "Visualization.h"

// ─── Helper: show a window scaled to 1/8 of the image's own dimensions ────────
static void showScaled(const std::string& name, const cv::Mat& img)
{
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    cv::resizeWindow(name, img.cols / 2, img.rows / 2);
    cv::imshow(name, img);
}

// ─── Process a single loaded frame ────────────────────────────────────────────
static void processFrame(const cv::Mat& frame, bool showMask = false)
{
    constexpr double SCALE = 0.35;
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(), SCALE, SCALE);
    cv::Mat display = resized.clone();

    // 1. Skin segmentation
    cv::Mat mask = segmentSkin(resized);
    // 2. Hand shape features
    HandFeatures features = analyzeHand(mask);
    // 3. Gesture classification
    std::string gesture = classifyGesture(features);
    // 4. Overlay results
    drawResults(display, features, gesture);

    // 5. Display
    showScaled("Gesture Recognition", display);
    if (showMask) {
        cv::Mat maskColor;
        cv::cvtColor(mask, maskColor, cv::COLOR_GRAY2BGR);
        showScaled("Skin Mask (debug)", maskColor);
    }

    // Console log for quick comparison across batch runs
    std::cout << "[Result] " << gesture
        << " | fingers=" << features.fingerCount
        << " solidity=" << features.solidity
        << " AR=" << features.aspectRatio
        << "\n";
}

// ─── Entry point ──────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    std::cout << "Hand Gesture Recognition\n";
    std::cout << "========================\n";

    std::vector<std::string> paths;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i)
            paths.emplace_back(argv[i]);
    }
    else {
        paths.emplace_back("thumbs_up.jpg");
    }

    constexpr bool SHOW_MASK = true;

    for (const auto& path : paths)
    {
        cv::Mat frame = cv::imread(path);
        if (frame.empty()) {
            std::cerr << "[Error] Could not load: " << path << "\n";
            continue;
        }

        std::cout << "\n[Image] " << path << "\n";
        processFrame(frame, SHOW_MASK);
        std::cout << "Press any key for next image (or to quit)...\n";
        cv::waitKey(0);
    }

    cv::destroyAllWindows();
    return 0;
}
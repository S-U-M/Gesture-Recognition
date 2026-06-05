#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "Constants.h"
#include "SkinSegmentation.h"
#include "HandAnalysis.h"
#include "GestureClassifier.h"
#include "Visualization.h"

/**
 * File: main.cpp
 * Project: Hand Gesture Recognition System
 * Author: Taylor Kang and Sumana Yeluripati
 *
 * Purpose:
 * Provides the command-line entry point for the project. This file loads test
 * images, runs the complete gesture recognition pipeline, displays the visual
 * output, and prints feature values to the console for debugging and comparison.
 *
 * Usage:
 * Project Test.exe image1.jpg image2.jpg image3.jpg
 *
 * Assumptions:
 * Input paths refer to readable image files. OpenCV 4 is correctly configured in
 * Visual Studio. Each input image contains a visible hand gesture that can be
 * separated from the background by the HSV segmentation stage.
 */

 /**
  * Displays an image in a resizable OpenCV window.
  *
  * Preconditions:
  * name is a non-empty window title. img is a valid OpenCV matrix.
  *
  * Postconditions:
  * A named OpenCV window is created or reused, resized to half of the image
  * dimensions, and updated with the given image.
  *
  * Parameters:
  * name: Title of the OpenCV display window.
  * img: Image to display.
  */
static void showScaled(const std::string& name, const cv::Mat& img)
{
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    cv::resizeWindow(name, img.cols / 2, img.rows / 2);
    cv::imshow(name, img);
}

/**
 * Runs the full recognition pipeline on a single loaded image.
 *
 * Preconditions:
 * frame is a valid non-empty BGR image. showMask controls whether the
 * intermediate skin mask is displayed for debugging.
 *
 * Postconditions:
 * The original frame is not modified. The processed result image and optional
 * mask are displayed. A one-line summary is printed to the console.
 *
 * Parameters:
 * frame: Input BGR image loaded from disk.
 * showMask: True to display the binary skin mask, false to hide it.
 */
static void processFrame(const cv::Mat& frame, bool showMask = false)
{
    // Resize the input to keep processing and display size manageable. This also
    // makes the demo easier to view when the test images are large.
    constexpr double SCALE = 0.35;
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(), SCALE, SCALE);
    cv::Mat display = resized.clone();

    // Stage 1: Convert the image into a binary mask of likely skin pixels.
    cv::Mat mask = segmentSkin(resized);

    // Stage 2: Extract contour, convex hull, solidity, aspect ratio, and finger
    // count from the mask.
    HandFeatures features = analyzeHand(mask);

    // Stage 3: Convert numeric hand features into a gesture label.
    std::string gesture = classifyGesture(features);

    // Stage 4: Draw contour, hull, bounding box, and label on the output image.
    drawResults(display, features, gesture);

    // Stage 5: Show the final result and, when requested, the intermediate mask.
    showScaled("Gesture Recognition", display);
    if (showMask) {
        cv::Mat maskColor;
        cv::cvtColor(mask, maskColor, cv::COLOR_GRAY2BGR);
        showScaled("Skin Mask (debug)", maskColor);
    }

    // Print measurable output values so the team can compare behavior across
    // test images and include quantitative observations in the write-up.
    std::cout << "[Result] " << gesture
        << " | fingers=" << features.fingerCount
        << " solidity=" << features.solidity
        << " AR=" << features.aspectRatio
        << "\n";
}

/**
 * Program entry point.
 *
 * Preconditions:
 * Command-line arguments, when present, are image paths. If no arguments are
 * given, the program tries to load thumbs_up.jpg from the working directory.
 *
 * Postconditions:
 * All readable input images are processed one at a time. Invalid image paths are
 * reported to the console without crashing the program. OpenCV windows are
 * destroyed before the program exits.
 *
 * Parameters:
 * argc: Number of command-line arguments.
 * argv: Array of command-line argument strings.
 *
 * Returns:
 * 0 when execution completes.
 */
int main(int argc, char* argv[])
{
    std::cout << "Hand Gesture Recognition\n";
    std::cout << "========================\n";

    // Collect all image paths from the command line. Multiple paths allow the
    // batch script to run every test image in one program launch.
    std::vector<std::string> paths;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i)
            paths.emplace_back(argv[i]);
    }
    else {
        paths.emplace_back("thumbsup1.jpg");
    }

    // Set to true for final testing because the mask helps explain the pipeline
    // during demonstrations and screenshots.
    constexpr bool SHOW_MASK = true;

    // Process each image independently. A failed image load is reported, then
    // the program continues to the next image instead of terminating.
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

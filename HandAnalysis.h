#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @brief All shape features extracted from the largest hand contour.
 *
 * fingerCount  – number of raised fingers detected via convexity defects.
 *                0 = fist, 1 = one finger/thumb, …, 5 = open palm.
 * aspectRatio  – bounding-rect width / height.
 *                < 1 → tall (portrait);  > 1 → wide (landscape).
 * solidity     – contour area / convex-hull area  (0–1).
 *                High solidity (~0.9+) → compact, closed hand.
 *                Low  solidity (~0.7)  → many gaps between fingers.
 * areaRatio    – contour area / bounding-rect area.
 * valid        – false when no usable hand blob was found.
 */
struct HandFeatures {
    int    fingerCount = 0;
    double aspectRatio = 1.0;
    double solidity = 0.0;
    double areaRatio = 0.0;
    bool   valid = false;

    cv::Rect               boundingBox;
    std::vector<cv::Point> contour;
    std::vector<cv::Point> hull;
};

/**
 * @brief Analyzes a binary skin mask and returns hand shape features.
 *
 * Steps:
 *   1. Find the largest external contour.
 *   2. Compute convex hull + convexity defects.
 *   3. Filter defects by depth and wrist-exclusion zone.
 *   4. Count finger gaps; convert gap count → finger count.
 *
 * @param mask  Binary skin mask produced by segmentSkin().
 * @return      Populated HandFeatures; .valid == false if no hand found.
 */
HandFeatures analyzeHand(const cv::Mat& mask);

/**
 * @brief Returns the angle (degrees) at vertex B in triangle A–B–C,
 *        computed via the dot-product form of the law of cosines.
 */
double angleBetween(const cv::Point& a, const cv::Point& b, const cv::Point& c);
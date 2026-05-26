#pragma once
#include <opencv2/opencv.hpp>
#include "HandAnalysis.h"
#include <string>

/**
 * @brief Overlays gesture recognition results onto a display frame.
 *
 * Draws:
 *   - Green contour outline
 *   - Blue convex hull
 *   - Light-purple bounding rectangle
 *   - Large gesture label (shadow + colored text)
 *   - Small debug line: finger count, solidity %, aspect ratio
 *
 * @param display  BGR image to draw on (modified in place).
 * @param f        HandFeatures from analyzeHand().
 * @param gesture  Label string from classifyGesture().
 */
void drawResults(cv::Mat& display, const HandFeatures& f, const std::string& gesture);
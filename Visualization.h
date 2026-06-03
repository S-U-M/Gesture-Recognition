#pragma once
#include <opencv2/opencv.hpp>
#include "HandAnalysis.h"
#include <string>

/**
 * File: Visualization.h
 * Project: Hand Gesture Recognition System
 * Author: Taylor Kang and Sumana Yeluripati
 *
 * Purpose:
 * Declares the display function used to draw gesture recognition results on an
 * image. This keeps visualization code separate from segmentation, analysis,
 * and classification logic.
 *
 * Assumptions:
 * The display image is a valid BGR OpenCV image, and the HandFeatures object
 * was produced by analyzeHand().
 */

 /**
  * Draws the detected contour, convex hull, bounding box, and gesture label.
  *
  * Preconditions:
  * display must be a valid BGR image. f should be the output of analyzeHand(),
  * and gesture should be the output of classifyGesture().
  *
  * Postconditions:
  * display is modified in place by drawing overlays. f and gesture are not
  * modified.
  *
  * Parameters:
  * display: Image where visual results are drawn.
  * f: Extracted hand features used for drawing contour, hull, and bounding box.
  * gesture: Text label to draw on the image.
  */
void drawResults(cv::Mat& display, const HandFeatures& f, const std::string& gesture);

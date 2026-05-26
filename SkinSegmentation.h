#pragma once
#include <opencv2/opencv.hpp>

/**
 * @brief Segments skin pixels from a BGR image using dual HSV ranges.
 *
 * Pipeline:
 *   BGR → HSV  →  inRange (two ranges)  →  erode/dilate  →  Gaussian blur
 *   →  threshold
 *
 * @param bgr  Input BGR image (unchanged).
 * @return     Single-channel binary mask: 255 = skin, 0 = background.
 */
cv::Mat segmentSkin(const cv::Mat& bgr);

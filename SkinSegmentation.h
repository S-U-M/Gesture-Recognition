#pragma once
#include <opencv2/opencv.hpp>

/**
 * File: SkinSegmentation.h
 * Project: Hand Gesture Recognition System
 * Author: Taylor Kang and Sumana Yeluripati
 *
 * Purpose:
 * Declares the skin segmentation function used at the beginning of the gesture
 * recognition pipeline. This function converts a BGR image into a binary mask
 * that separates likely skin pixels from the background.
 *
 * Assumptions:
 * The input image is a valid OpenCV BGR image. The HSV thresholds are tuned for
 * the project's test conditions and may need adjustment for different lighting,
 * backgrounds, or cameras.
 */

 /**
  * Segments likely skin pixels from a BGR image using two HSV threshold ranges.
  *
  * Preconditions:
  * bgr must be a non-empty three-channel image in OpenCV BGR format.
  *
  * Postconditions:
  * The input image is not modified. A new single-channel binary mask is returned.
  * Skin-like pixels are represented by 255, and background pixels are represented
  * by 0.
  *
  * Parameters:
  * bgr: Input color image in BGR format.
  *
  * Returns:
  * Binary skin mask suitable for contour detection and hand analysis.
  */
cv::Mat segmentSkin(const cv::Mat& bgr);

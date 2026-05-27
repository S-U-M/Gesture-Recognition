#pragma once
#include <opencv2/opencv.hpp>

// HSV skin ranges. OpenCV hue range is 0..180.
// The lower saturation bound is intentionally moderate so bright indoor lighting
// does not erase the hand.
const cv::Scalar SKIN_LOW(0, 25, 50);
const cv::Scalar SKIN_HIGH(25, 255, 255);
const cv::Scalar SKIN_LOW2(160, 25, 50);
const cv::Scalar SKIN_HIGH2(180, 255, 255);

// YCrCb skin range used together with HSV to make segmentation less lighting-sensitive.
const cv::Scalar SKIN_YCRCB_LOW(0, 133, 77);
const cv::Scalar SKIN_YCRCB_HIGH(255, 173, 127);

// Contour filtering.
// Use both an absolute floor and an image-relative floor so resizing does not break detection.
const double MIN_HAND_AREA = 1200.0;
const double MIN_HAND_AREA_RATIO = 0.003;

// Convexity-defect thresholds.
const double MAX_DEFECT_ANGLE_DEG = 115.0;
const double MIN_DEFECT_DEPTH = 10.0;
const double MIN_DEFECT_DEPTH_RATIO = 0.025;

// Defects below this fraction of the bounding box are treated as wrist/forearm valleys.
const double WRIST_CUTOFF_FRACTION = 0.78;

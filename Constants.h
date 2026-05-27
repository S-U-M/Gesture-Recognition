#pragma once
#include <opencv2/opencv.hpp>

// ─── HSV skin ranges ──────────────────────────────────────────────────────────
// Primary range covers light-to-medium skin under indoor lighting
const cv::Scalar SKIN_LOW(0, 20, 70);
const cv::Scalar SKIN_HIGH(20, 255, 255);

// Secondary range captures darker/redder hues near the red wraparound
const cv::Scalar SKIN_LOW2(170, 20, 70);
const cv::Scalar SKIN_HIGH2(180, 255, 255);

// ─── Contour / hand filtering ─────────────────────────────────────────────────
// Blobs below this area (px²) are ignored as background noise
const double MIN_HAND_AREA = 8000.0;

// ─── Convexity-defect thresholds ──────────────────────────────────────────────
// Valleys with an opening angle above this are NOT finger gaps
// (raised to 90° — tight enough to reject wrist concavities)
const double MAX_DEFECT_ANGLE_DEG = 120.0;

// Raised from 20 → 40 px to suppress shallow hull artifacts and wrist edges
const double MIN_DEFECT_DEPTH = 60.0;

// Defects whose far-point sits below this fraction of the bounding box
// are assumed to be wrist valleys and are discarded
const double WRIST_CUTOFF_FRACTION = 0.60;
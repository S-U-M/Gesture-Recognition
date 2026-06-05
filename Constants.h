#pragma once
#include <opencv2/opencv.hpp>

/**
 * File: Constants.h
 * Project: Hand Gesture Recognition System
 * Author: Taylor Kang and Sumana Yeluripati
 *
 * Purpose:
 * Defines shared threshold values used by the skin segmentation and hand
 * analysis stages. Keeping these constants in one header makes the classifier
 * easier to tune because HSV limits, contour limits, and convexity defect
 * thresholds can be adjusted without searching through multiple source files.
 *
 * Assumptions:
 * The input images are taken under reasonably consistent lighting and contain
 * one dominant hand region. The threshold values were selected experimentally
 * for the project's test images and may need tuning for different cameras,
 * backgrounds, or lighting conditions.
 */

 // Primary HSV range for skin-like pixels under common indoor lighting.
 // H is hue, S is saturation, and V is brightness in OpenCV's HSV format.
const cv::Scalar SKIN_LOW(0, 20, 70);
const cv::Scalar SKIN_HIGH(20, 255, 255);

// Secondary HSV range for red-wraparound hue values.
// This helps keep reddish or darker skin regions from being excluded.
const cv::Scalar SKIN_LOW2(170, 20, 70);
const cv::Scalar SKIN_HIGH2(180, 255, 255);

// Minimum contour area, in pixels squared, required to treat a blob as a hand.
// Smaller blobs are ignored because they are usually background noise.
const double MIN_HAND_AREA = 8000.0;

// Maximum allowed angle, in degrees, for a strict convexity defect to count as
// a finger gap. Wider angles are more likely to be wrist or contour artifacts.
const double MAX_DEFECT_ANGLE_DEG = 120.0;

// Minimum depth, in pixels, for a strict convexity defect to count as a finger
// gap. Raising this value suppresses shallow defects caused by rough contours.
const double MIN_DEFECT_DEPTH = 60.0;

// Fraction of the bounding box height used to exclude wrist and forearm defects.
// Defect far-points below this cutoff are ignored during finger counting.
const double WRIST_CUTOFF_FRACTION = 0.60;

// Wider angle tolerance used only for looser inter-finger gap detection.
// This is useful for gestures such as Peace Sign, where the visible gap may be
// wide and may not satisfy the stricter defect rule.
const double MAX_FINGER_ANGLE_DEG = 145.0;

// Lower depth threshold used for wide-angle defects in two-finger gestures.
// This helps detect Peace Sign cases where the gap is visible but shallow.
const double MIN_PEACE_DEFECT_DEPTH = 10.0;

#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * File: HandAnalysis.h
 * Project: Hand Gesture Recognition System
 * Author: Taylor Kang and Sumana Yeluripati
 *
 * Purpose:
 * Declares the data structure and functions used to extract hand-shape
 * features from a binary skin mask. This module converts low-level contour
 * geometry into higher-level values such as finger count, aspect ratio,
 * solidity, area ratio, convex hull, and bounding box data.
 *
 * Assumptions:
 * The input mask is a single-channel binary image where hand pixels are white
 * and background pixels are black. The largest valid external contour in the
 * mask is assumed to be the hand.
 */

 /**
  * Stores all hand-shape features extracted from the largest detected contour.
  *
  * Field meanings:
  * fingerCount:
  * Number of raised fingers estimated from convexity defects.
  * 0 may indicate a fist or no detected finger gaps, 1 may indicate one
  * finger/thumb, and values up to 5 may indicate an open palm.
  *
  * aspectRatio:
  * Bounding rectangle width divided by height.
  * A value less than 1 means the hand region is taller than it is wide.
  * A value greater than 1 means the hand region is wider than it is tall.
  *
  * solidity:
  * Contour area divided by convex hull area, usually between 0 and 1.
  * High solidity, around 0.9 or higher, usually means the hand shape is compact
  * or closed. Lower solidity, around 0.7, usually means there are larger gaps
  * between fingers.
  *
  * areaRatio:
  * Contour area divided by bounding rectangle area.
  *
  * valid:
  * True only when a contour large enough to be treated as a hand is found.
  * False when no usable hand blob was found.
  *
  * boundingBox:
  * Rectangle enclosing the detected hand contour.
  *
  * contour:
  * Largest external contour selected as the hand.
  *
  * hull:
  * Point-based convex hull used for visualization and solidity calculation.
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
 * Analyzes a binary skin mask and returns hand shape features.
 *
 * Processing steps:
 * 1. Find all external contours in the binary mask.
 * 2. Select the largest contour as the most likely hand region.
 * 3. Reject the contour if its area is smaller than MIN_HAND_AREA.
 * 4. Compute the bounding rectangle, aspect ratio, area ratio, and validity.
 * 5. Compute the convex hull for drawing and solidity calculation.
 * 6. Compute convexity defects from the index-based convex hull.
 * 7. Filter defects by depth, angle, and wrist-exclusion zone.
 * 8. Count valid finger gaps and convert the gap count into a finger estimate.
 *
 * Preconditions:
 * mask must be a non-empty, single-channel binary OpenCV matrix. White pixels
 * should represent likely skin or hand regions, and black pixels should
 * represent the background.
 *
 * Postconditions:
 * The input mask is not intentionally modified. The returned HandFeatures
 * object has valid set to true only if a contour larger than MIN_HAND_AREA is
 * found. If no usable hand contour exists, valid remains false and default
 * feature values are returned.
 *
 * Parameters:
 * mask: Binary skin mask produced by segmentSkin().
 *
 * Returns:
 * A populated HandFeatures object containing contour, hull, bounding box,
 * finger count, aspect ratio, solidity, and area ratio information. If no hand
 * is found, the returned object has valid set to false.
 */
HandFeatures analyzeHand(const cv::Mat& mask);

/**
 * Computes the angle at point b in triangle a-b-c.
 *
 * The angle is computed in degrees using the dot-product form of the law of
 * cosines. This helper is used when filtering convexity defects so that wide
 * wrist or palm concavities are not incorrectly counted as finger gaps.
 *
 * Preconditions:
 * a, b, and c are valid OpenCV points. Degenerate line segments are handled by
 * returning 180 degrees.
 *
 * Postconditions:
 * No input points are modified.
 *
 * Parameters:
 * a: First endpoint of the triangle angle.
 * b: Vertex point where the angle is measured.
 * c: Second endpoint of the triangle angle.
 *
 * Returns:
 * Angle in degrees at vertex b.
 */
double angleBetween(const cv::Point& a, const cv::Point& b, const cv::Point& c);
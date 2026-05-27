#include "HandAnalysis.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>

static double safeRectArea(const cv::Rect& r)
{
    return static_cast<double>(std::max(1, r.width)) * static_cast<double>(std::max(1, r.height));
}

double angleBetween(const cv::Point& a, const cv::Point& b, const cv::Point& c)
{
    double ba_x = a.x - b.x;
    double ba_y = a.y - b.y;
    double bc_x = c.x - b.x;
    double bc_y = c.y - b.y;

    double dot = ba_x * bc_x + ba_y * bc_y;
    double magBA = std::sqrt(ba_x * ba_x + ba_y * ba_y);
    double magBC = std::sqrt(bc_x * bc_x + bc_y * bc_y);

    if (magBA < 1e-6 || magBC < 1e-6) {
        return 180.0;
    }

    double cosA = dot / (magBA * magBC);
    cosA = std::max(-1.0, std::min(1.0, cosA));
    return std::acos(cosA) * 180.0 / CV_PI;
}

HandFeatures analyzeHand(const cv::Mat& mask)
{
    HandFeatures f;

    if (mask.empty()) {
        return f;
    }

    cv::Mat cleanMask = mask.clone();

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(cleanMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) {
        return f;
    }

    const double imageArea = static_cast<double>(mask.rows) * static_cast<double>(mask.cols);
    const double minArea = std::max(MIN_HAND_AREA, imageArea * MIN_HAND_AREA_RATIO);

    int bestIndex = -1;
    double bestArea = 0.0;

    for (int i = 0; i < static_cast<int>(contours.size()); ++i) {
        double area = cv::contourArea(contours[i]);
        if (area < minArea) {
            continue;
        }

        cv::Rect box = cv::boundingRect(contours[i]);
        if (box.width < 25 || box.height < 25) {
            continue;
        }

        if (area > bestArea) {
            bestArea = area;
            bestIndex = i;
        }
    }

    if (bestIndex < 0) {
        return f;
    }

    std::vector<cv::Point> contour = contours[bestIndex];

    // Smooth tiny segmentation notches without destroying the hand outline.
    double perimeter = cv::arcLength(contour, true);
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contour, approx, 0.0025 * perimeter, true);
    if (approx.size() >= 5) {
        contour = approx;
    }

    double area = cv::contourArea(contour);
    if (area < minArea) {
        return f;
    }

    f.contour = contour;
    f.boundingBox = cv::boundingRect(f.contour);
    f.aspectRatio = static_cast<double>(f.boundingBox.width) / static_cast<double>(std::max(1, f.boundingBox.height));
    f.areaRatio = area / safeRectArea(f.boundingBox);
    f.valid = true;

    std::vector<int> hullIdx;
    std::vector<cv::Point> hullPts;
    cv::convexHull(f.contour, hullIdx, false, false);
    cv::convexHull(f.contour, hullPts, false, true);
    f.hull = hullPts;

    double hullArea = cv::contourArea(hullPts);
    f.solidity = (hullArea > 1.0) ? area / hullArea : 0.0;

    if (hullIdx.size() <= 3 || f.contour.size() <= 3) {
        return f;
    }

    std::vector<cv::Vec4i> defects;
    cv::convexityDefects(f.contour, hullIdx, defects);

    int wristCutoffY = f.boundingBox.y + static_cast<int>(f.boundingBox.height * WRIST_CUTOFF_FRACTION);
    double minDepth = std::max(MIN_DEFECT_DEPTH, f.boundingBox.height * MIN_DEFECT_DEPTH_RATIO);

    int validDefects = 0;

    for (const auto& d : defects) {
        cv::Point start = f.contour[d[0]];
        cv::Point end = f.contour[d[1]];
        cv::Point far = f.contour[d[2]];
        double depth = d[3] / 256.0;

        if (depth < minDepth) {
            continue;
        }

        if (far.y > wristCutoffY) {
            continue;
        }

        double angle = angleBetween(start, far, end);
        if (angle > MAX_DEFECT_ANGLE_DEG) {
            continue;
        }

        // Ignore tiny edge defects near the side of the bounding box.
        int leftLimit = f.boundingBox.x + static_cast<int>(0.08 * f.boundingBox.width);
        int rightLimit = f.boundingBox.x + static_cast<int>(0.92 * f.boundingBox.width);
        if (far.x < leftLimit || far.x > rightLimit) {
            continue;
        }

        ++validDefects;
    }

    if (validDefects > 0) {
        f.fingerCount = std::min(5, validDefects + 1);
    }
    else {
        // Convexity defects cannot reliably distinguish a fist from a single raised
        // thumb/finger, because both can produce zero valid defects. Use compactness.
        bool elongated = (f.aspectRatio < 0.78 || f.aspectRatio > 1.35);
        bool notCompactFist = (f.areaRatio < 0.72 || f.solidity < 0.92);

        if (elongated && notCompactFist) {
            f.fingerCount = 1;
        }
        else {
            f.fingerCount = 0;
        }
    }

    return f;
}

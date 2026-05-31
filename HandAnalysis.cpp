#include "HandAnalysis.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>

// ─── Utility ──────────────────────────────────────────────────────────────────

double angleBetween(const cv::Point& a, const cv::Point& b, const cv::Point& c)
{
    double ba_x = a.x - b.x, ba_y = a.y - b.y;
    double bc_x = c.x - b.x, bc_y = c.y - b.y;

    double dot = ba_x * bc_x + ba_y * bc_y;
    double magBA = std::sqrt(ba_x * ba_x + ba_y * ba_y);
    double magBC = std::sqrt(bc_x * bc_x + bc_y * bc_y);

    if (magBA < 1e-6 || magBC < 1e-6) return 180.0;

    double cosA = dot / (magBA * magBC);
    cosA = std::max(-1.0, std::min(1.0, cosA));   // clamp against FP rounding
    return std::acos(cosA) * 180.0 / CV_PI;
}

// ─── Main analysis ────────────────────────────────────────────────────────────

HandFeatures analyzeHand(const cv::Mat& mask)
{
    HandFeatures f;

    // 1. Find all external contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return f;

    // 2. Keep only the largest contour by area
    auto maxIt = std::max_element(contours.begin(), contours.end(),
        [](const auto& a, const auto& b) {
            return cv::contourArea(a) < cv::contourArea(b);
        });

    double area = cv::contourArea(*maxIt);
    if (area < MIN_HAND_AREA) return f;

    f.contour = *maxIt;
    f.boundingBox = cv::boundingRect(f.contour);
    f.aspectRatio = (double)f.boundingBox.width / (double)f.boundingBox.height;
    f.areaRatio = area / (double)(f.boundingBox.width * f.boundingBox.height);
    f.valid = true;

    // 3. Convex hull — two flavors:
    //    hullIdx  (index-based) → required by convexityDefects()
    //    hullPts  (point-based) → used for drawing and area computation
    std::vector<int>       hullIdx;
    std::vector<cv::Point> hullPts;
    cv::convexHull(f.contour, hullIdx, false, false);
    cv::convexHull(f.contour, hullPts, false, true);
    f.hull = hullPts;

    double hullArea = cv::contourArea(hullPts);
    f.solidity = (hullArea > 1.0) ? area / hullArea : 0.0;

    // 4. Convexity defects → finger-gap detection
    if (hullIdx.size() <= 3) return f;   // degenerate hull, skip

    std::vector<cv::Vec4i> defects;
    cv::convexityDefects(f.contour, hullIdx, defects);

    // --- FIX: wrist-exclusion cutoff ----------------------------------------
    // Defects whose far-point lies in the bottom WRIST_CUTOFF_FRACTION of the
    // bounding box are almost always wrist/forearm concavities, not finger gaps.
    // Discarding them prevents a thumbs-up being mis-counted as 2 fingers.
    int wristCutoffY = f.boundingBox.y
        + static_cast<int>(f.boundingBox.height * WRIST_CUTOFF_FRACTION);

    int validDefects = 0;
    int wideDefects = 0;    // ← new

    for (const auto& d : defects)
    {
        cv::Point start = f.contour[d[0]];
        cv::Point end = f.contour[d[1]];
        cv::Point far = f.contour[d[2]];
        double    depth = d[3] / 256.0;

        // AFTER
        if (far.y > wristCutoffY) continue;

        double angle = angleBetween(start, far, end);

        if (depth >= MIN_DEFECT_DEPTH && angle < MAX_DEFECT_ANGLE_DEG)
            ++validDefects;
        else if (depth >= MIN_PEACE_DEFECT_DEPTH && angle < MAX_FINGER_ANGLE_DEG)
            ++wideDefects;

        printf("depth=%.1f  angle=%.1f  far.y=%d  cutoffY=%d\n",
            depth, angleBetween(start, far, end), far.y, wristCutoffY);

    }

    /*if (validDefects == 1)
        f.fingerCount = 1;
    else if (validDefects > 1)
        f.fingerCount = std::min(5, validDefects + 1);*/

    // ← if no strict defects but wide ones found, likely a peace sign
    if (validDefects == 1 && wideDefects >= 3)
        f.fingerCount = std::min(5, 1 + wideDefects + 1);
    else if (validDefects == 1)
        f.fingerCount = 1;
    else if (validDefects > 1)
        f.fingerCount = std::min(5, validDefects + 1);
    else if (validDefects == 0 && wideDefects >= 1)
        f.fingerCount = 2;
    // else f.fingerCount stays 0  →  correctly classified as Fist

    return f;
}
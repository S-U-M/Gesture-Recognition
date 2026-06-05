#include "HandAnalysis.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>

/**
 * Computes the angle at point b using the dot product formula.
 *
 * Preconditions:
 * a, b, and c are valid OpenCV points. If either vector has near-zero length,
 * the function treats the angle as 180 degrees so the caller will not count it
 * as a strong finger gap.
 *
 * Postconditions:
 * No input values are modified.
 */
double angleBetween(const cv::Point& a, const cv::Point& b, const cv::Point& c)
{
    // Build vectors BA and BC because the angle is measured at vertex b.
    double ba_x = a.x - b.x, ba_y = a.y - b.y;
    double bc_x = c.x - b.x, bc_y = c.y - b.y;

    // Compute dot product and vector lengths for the cosine formula.
    double dot = ba_x * bc_x + ba_y * bc_y;
    double magBA = std::sqrt(ba_x * ba_x + ba_y * ba_y);
    double magBC = std::sqrt(bc_x * bc_x + bc_y * bc_y);

    // Avoid division by zero for degenerate triangles.
    if (magBA < 1e-6 || magBC < 1e-6) return 180.0;

    // Clamp the cosine value to protect acos() from floating-point rounding
    // errors that could produce a value slightly outside [-1, 1].
    double cosA = dot / (magBA * magBC);
    cosA = std::max(-1.0, std::min(1.0, cosA));
    return std::acos(cosA) * 180.0 / CV_PI;
}

/**
 * Extracts contour-based hand features from a binary skin mask.
 *
 * Preconditions:
 * mask is a binary image where white pixels represent likely hand or skin
 * regions. The image should already be cleaned by segmentSkin().
 *
 * Postconditions:
 * The returned HandFeatures object is marked valid only if a large enough
 * contour is found. The input mask is not intentionally modified.
 */
HandFeatures analyzeHand(const cv::Mat& mask)
{
    // Start with default feature values. If any stage fails, this object is
    // returned with valid set to false or with a conservative finger count.
    HandFeatures f;

    // Find external contours so nested holes or internal gaps do not become
    // separate hand candidates.
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return f;

    // Choose the largest contour because the project assumes one dominant hand
    // region in the input image.
    auto maxIt = std::max_element(contours.begin(), contours.end(),
        [](const auto& a, const auto& b) {
            return cv::contourArea(a) < cv::contourArea(b);
        });

    // Reject very small blobs so background noise is not treated as a hand.
    double area = cv::contourArea(*maxIt);
    if (area < MIN_HAND_AREA) return f;

    // Save basic contour and bounding rectangle measurements used by both the
    // classifier and the visualization stage.
    f.contour = *maxIt;
    f.boundingBox = cv::boundingRect(f.contour);
    f.aspectRatio = (double)f.boundingBox.width / (double)f.boundingBox.height;
    f.areaRatio = area / (double)(f.boundingBox.width * f.boundingBox.height);
    f.valid = true;

    // Compute two versions of the convex hull. The index-based hull is required
    // by convexityDefects(), while the point-based hull is used for drawing and
    // for calculating solidity.
    std::vector<int>       hullIdx;
    std::vector<cv::Point> hullPts;
    cv::convexHull(f.contour, hullIdx, false, false);
    cv::convexHull(f.contour, hullPts, false, true);
    f.hull = hullPts;

    // Solidity measures how much of the convex hull is filled by the contour.
    // Spread fingers reduce solidity because they create empty gaps in the hull.
    double hullArea = cv::contourArea(hullPts);
    f.solidity = (hullArea > 1.0) ? area / hullArea : 0.0;

    // A hull with three or fewer points cannot produce meaningful convexity
    // defects, so return the features collected so far.
    if (hullIdx.size() <= 3) return f;

    // Convexity defects identify concave gaps between the hand contour and its
    // convex hull. These gaps are the main evidence used for finger counting.
    std::vector<cv::Vec4i> defects;
    cv::convexityDefects(f.contour, hullIdx, defects);

    // Exclude the lower part of the bounding box because wrist and forearm
    // concavities often look like finger gaps to convexityDefects().
    int wristCutoffY = f.boundingBox.y
        + static_cast<int>(f.boundingBox.height * WRIST_CUTOFF_FRACTION);

    // validDefects stores strict finger gaps. wideDefects stores looser gaps
    // that are useful for recognizing peace signs with shallow or wide valleys.
    int validDefects = 0;
    int wideDefects = 0;

    for (const auto& d : defects)
    {
        // Each defect contains contour indices for the start point, end point,
        // farthest concave point, and defect depth in fixed-point format.
        cv::Point start = f.contour[d[0]];
        cv::Point end = f.contour[d[1]];
        cv::Point far = f.contour[d[2]];
        double    depth = d[3] / 256.0;

        // Ignore concavities below the wrist cutoff because they are more likely
        // to come from the wrist or forearm than from fingers.
        if (far.y > wristCutoffY) continue;

        // The angle at the far point helps distinguish narrow finger gaps from
        // broad contour curves.
        double angle = angleBetween(start, far, end);

        // Count strict defects first. If a defect is too shallow or too wide for
        // strict counting, allow a looser rule for possible two-finger gestures.
        if (depth >= MIN_DEFECT_DEPTH && angle < MAX_DEFECT_ANGLE_DEG)
            ++validDefects;
        else if (depth >= MIN_PEACE_DEFECT_DEPTH && angle < MAX_FINGER_ANGLE_DEG)
            ++wideDefects;

        // Debug output used during threshold tuning. It is useful for the write-up
        // and can be removed if the final demo should show only gesture labels.
        printf("depth=%.1f  angle=%.1f  far.y=%d  cutoffY=%d\n",
            depth, angleBetween(start, far, end), far.y, wristCutoffY);
    }

    // Convert defect counts to an estimated finger count. The mapping is
    // approximate because convexity defects measure gaps, not fingers directly.
    if (validDefects == 1 && wideDefects >= 3)
        f.fingerCount = std::min(5, 1 + wideDefects + 1);
    else if (validDefects == 1)
        f.fingerCount = 1;
    else if (validDefects > 1)
        f.fingerCount = std::min(5, validDefects + 1);
    else if (validDefects == 0 && wideDefects >= 1)
        f.fingerCount = 2;

    // If no valid or wide defects were found, fingerCount remains 0. The
    // classifier later treats that as an unknown or closed-hand-like case.
    return f;
}

/*
 * Hand Gesture Recognition Prototype
 * -----------------------------------
 * Approach:
 *   1. Convert RGB -> HSV, isolate skin tones via inRange()
 *   2. Morphological cleanup (erode + dilate)
 *   3. Find largest contour (the hand)
 *   4. Compute convex hull + convexity defects to count fingers
 *   5. Classify gesture from finger count + shape features
 *
 * Supported gestures:
 *   Fist, Open Palm, Thumbs Up, Peace Sign, Okay Sign, Stop Gesture
 *
 * Build: Visual Studio 2022, OpenCV 4.x
 * Usage:
 *   gesture_recognition.exe image.jpg        <- single image
 *   gesture_recognition.exe img1.jpg img2.jpg ... <- batch images
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

 // ─── Tuneable constants ────────────────────────────────────────────────────────

 // HSV skin range (works for most skin tones under indoor lighting)
 // Adjust these if segmentation looks off on your images
const cv::Scalar SKIN_LOW(0, 20, 70);
const cv::Scalar SKIN_HIGH(20, 255, 255);

// Second HSV range for darker/redder skin tones
const cv::Scalar SKIN_LOW2(170, 20, 70);
const cv::Scalar SKIN_HIGH2(180, 255, 255);

// Minimum contour area to consider as a hand (filters small noise blobs)
const double MIN_HAND_AREA = 8000.0;

// Convexity defect angle threshold: valleys deeper than this are finger gaps
const double MAX_DEFECT_ANGLE_DEG = 90.0;

// Minimum defect depth (in pixels) — filters shallow hull artifacts
const double MIN_DEFECT_DEPTH = 20.0;

// ─── Utilities ─────────────────────────────────────────────────────────────────

// Angle at vertex B of triangle A-B-C (law of cosines), in degrees
double angleBetween(const cv::Point& a, const cv::Point& b, const cv::Point& c)
{
    double ba_x = a.x - b.x, ba_y = a.y - b.y;
    double bc_x = c.x - b.x, bc_y = c.y - b.y;
    double dot = ba_x * bc_x + ba_y * bc_y;
    double magBA = std::sqrt(ba_x * ba_x + ba_y * ba_y);
    double magBC = std::sqrt(bc_x * bc_x + bc_y * bc_y);
    if (magBA < 1e-6 || magBC < 1e-6) return 180.0;
    double cosA = dot / (magBA * magBC);
    cosA = std::max(-1.0, std::min(1.0, cosA));   // clamp for numerical safety
    return std::acos(cosA) * 180.0 / CV_PI;
}

// ─── Skin segmentation ────────────────────────────────────────────────────────

cv::Mat segmentSkin(const cv::Mat& bgr)
{
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask1, mask2;
    cv::inRange(hsv, SKIN_LOW, SKIN_HIGH, mask1);
    cv::inRange(hsv, SKIN_LOW2, SKIN_HIGH2, mask2);
    cv::Mat mask = mask1 | mask2;

    // Morphological cleanup: remove noise, fill small holes
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, { 5, 5 });
    cv::erode(mask, mask, kernel, { -1,-1 }, 2);
    cv::dilate(mask, mask, kernel, { -1,-1 }, 4);

    // Optional: Gaussian blur then re-threshold to smooth jagged edges
    cv::GaussianBlur(mask, mask, { 5, 5 }, 0);
    cv::threshold(mask, mask, 127, 255, cv::THRESH_BINARY);

    return mask;
}

// ─── Hand analysis ────────────────────────────────────────────────────────────

struct HandFeatures {
    int    fingerCount = 0;   // number of raised fingers (0-5)
    double aspectRatio = 1.0; // bounding rect width/height
    double solidity = 0.0; // contour area / hull area
    double areaRatio = 0.0; // contour area / bounding rect area
    bool   valid = false;
    cv::Rect boundingBox;
    std::vector<cv::Point> contour;
    std::vector<cv::Point> hull;
};

HandFeatures analyzeHand(const cv::Mat& mask)
{
    HandFeatures f;

    // Find all external contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return f;

    // Keep only the largest contour
    auto maxIt = std::max_element(contours.begin(), contours.end(),
        [](const auto& a, const auto& b) { return cv::contourArea(a) < cv::contourArea(b); });

    double area = cv::contourArea(*maxIt);
    if (area < MIN_HAND_AREA) return f;

    f.contour = *maxIt;
    f.boundingBox = cv::boundingRect(f.contour);
    f.aspectRatio = (double)f.boundingBox.width / (double)f.boundingBox.height;
    f.areaRatio = area / (double)(f.boundingBox.width * f.boundingBox.height);
    f.valid = true;

    // Convex hull (point indices for defect analysis, + point version for drawing)
    std::vector<int>        hullIdx;
    std::vector<cv::Point>  hullPts;
    cv::convexHull(f.contour, hullIdx, false, false);
    cv::convexHull(f.contour, hullPts, false, true);
    f.hull = hullPts;

    double hullArea = cv::contourArea(hullPts);
    f.solidity = (hullArea > 1.0) ? area / hullArea : 0.0;

    // Convexity defects = valleys between fingers
    if (hullIdx.size() > 3)
    {
        std::vector<cv::Vec4i> defects;
        cv::convexityDefects(f.contour, hullIdx, defects);

        for (const auto& d : defects)
        {
            cv::Point start = f.contour[d[0]];
            cv::Point end = f.contour[d[1]];
            cv::Point far = f.contour[d[2]];
            double    depth = d[3] / 256.0;   // depth is stored * 256

            if (depth < MIN_DEFECT_DEPTH) continue;

            double angle = angleBetween(start, far, end);
            if (angle < MAX_DEFECT_ANGLE_DEG)
                f.fingerCount++;
        }
        // defects ≈ gaps between fingers; fingers = gaps + 1 (clamped 0-5)
        f.fingerCount = std::min(5, f.fingerCount + 1);
    }

    return f;
}

// ─── Gesture classification ────────────────────────────────────────────────────

/*
 * Decision logic (simple rule-based — good enough for a prototype):
 *
 *  fingerCount | solidity | aspectRatio | gesture
 *  ------------+----------+-------------+---------------------
 *      0       |  high    |    any      | Fist
 *      1       |  any     |  tall       | Thumbs Up  (thumb only, portrait shape)
 *      2       |  any     |    any      | Peace Sign
 *      4 or 5  |  any     |  wide       | Stop / Open Palm (wide spread)
 *      4 or 5  |  any     |  tall       | Open Palm (upright)
 *      3       |  any     |    any      | Okay (rough — finger circle + 3 up)
 *              |          |             | (improve later with landmarks)
 */
std::string classifyGesture(const HandFeatures& f)
{
    if (!f.valid)
        return "No hand detected";

    int fc = f.fingerCount;
    double ar = f.aspectRatio;
    double sol = f.solidity;

    // Closed hand
    if (fc <= 0)
        return "Fist";

    // One raised finger/thumb
    if (fc == 1)
    {
        if (ar < 0.75)
            return "Thumbs Up";

        return "Pointing";
    }

    // Peace sign
    if (fc == 2)
        return "Peace Sign";

    // Open hand / stop
    if (fc >= 4)
    {
        if (ar > 0.9)
            return "Stop Gesture";

        return "Open Palm";
    }

    // Maybe okay sign
    if (fc == 3 && sol < 0.8)
        return "Okay Sign";

    return "Unknown";
}

// ─── Visualization ────────────────────────────────────────────────────────────

void drawResults(cv::Mat& display, const HandFeatures& f, const std::string& gesture)
{
    if (!f.valid) {
        cv::putText(display, "No hand detected", { 20, 40 },
            cv::FONT_HERSHEY_SIMPLEX, 1.2, { 0, 0, 200 }, 2);
        return;
    }

    // Draw contour
    std::vector<std::vector<cv::Point>> contours = { f.contour };
    cv::drawContours(display, contours, 0, { 0, 200, 0 }, 2);

    // Draw convex hull
    std::vector<std::vector<cv::Point>> hulls = { f.hull };
    cv::drawContours(display, hulls, 0, { 255, 100, 0 }, 2);

    // Draw bounding rect
    cv::rectangle(display, f.boundingBox, { 100, 100, 255 }, 2);

    // Gesture label (large, top-left)
    std::string label = gesture;

    cv::putText(display, label, { 40, 120 },
        cv::FONT_HERSHEY_SIMPLEX,
        4.0,                  // MUCH larger font
        { 0, 0, 0 },
        10);                  // thick shadow

    cv::putText(display, label, { 40, 120 },
        cv::FONT_HERSHEY_SIMPLEX,
        4.0,
        { 50, 200, 50 },
        5);                   // thicker text

    // Debug info (smaller, below label)
    std::string info = "Fingers: " + std::to_string(f.fingerCount)
        + "  Solidity: " + std::to_string((int)(f.solidity * 100)) + "%"
        + "  AR: " + std::to_string(f.aspectRatio).substr(0, 4);
    cv::putText(display, info, { 20, 90 },
        cv::FONT_HERSHEY_SIMPLEX, 0.6, { 0, 0, 0 }, 3);
    cv::putText(display, info, { 20, 90 },
        cv::FONT_HERSHEY_SIMPLEX, 0.6, { 200, 200, 200 }, 1);
}

// ─── Process a single frame ────────────────────────────────────────────────────

void processFrame(const cv::Mat& frame, bool showMask = false)
{
    // Resize frame FIRST
    double scale = 0.35; // adjust this value as needed
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(), scale, scale);

    cv::Mat display = resized.clone();

    // 1. Segment skin
    cv::Mat mask = segmentSkin(resized);

    // 2. Analyze hand
    HandFeatures features = analyzeHand(mask);

    // 3. Classify
    std::string gesture = classifyGesture(features);

    // 4. Draw
    drawResults(display, features, gesture);

    // Show windows
    cv::imshow("Gesture Recognition", display);

    if (showMask) {
        cv::Mat maskColor;
        cv::cvtColor(mask, maskColor, cv::COLOR_GRAY2BGR);
        cv::imshow("Skin Mask (debug)", maskColor);
    }

    std::cout << "[Result] " << gesture
        << " | fingers=" << features.fingerCount
        << " solidity=" << features.solidity
        << " AR=" << features.aspectRatio
        << "\n";
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    std::cout << "Hand Gesture Recognition Prototype\n";
    std::cout << "===================================\n";

    bool showMask = true;

    // ── Quick image test ──────────────────────────────────────────────────
    std::string path = "thumbs_up.jpg";
    cv::Mat frame = cv::imread(path);

    if (frame.empty()) {
        std::cerr << "Could not load image: " << path << "\n";
        std::cerr << "Make sure thumbs_up.jpeg is in the right folder.\n";
        return 1;
    }

    processFrame(frame, showMask);

    std::cout << "Press any key to quit...\n";
    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}
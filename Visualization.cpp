#include "Visualization.h"
#include <string>

void drawResults(cv::Mat& display, const HandFeatures& f, const std::string& gesture)
{
    if (!f.valid) {
        cv::putText(display, "No hand detected", { 20, 40 },
            cv::FONT_HERSHEY_SIMPLEX, 1.2, { 0, 0, 200 }, 2);
        return;
    }

    // ── Contour (green) ───────────────────────────────────────────────────────
    std::vector<std::vector<cv::Point>> contours = { f.contour };
    cv::drawContours(display, contours, 0, { 0, 200, 0 }, 2);

    // ── Convex hull (blue-orange) ─────────────────────────────────────────────
    std::vector<std::vector<cv::Point>> hulls = { f.hull };
    cv::drawContours(display, hulls, 0, { 255, 100, 0 }, 2);

    // ── Bounding rectangle (light purple) ────────────────────────────────────
    cv::rectangle(display, f.boundingBox, { 100, 100, 255 }, 2);

    // ── Large gesture label (shadow pass + colour pass) ───────────────────────
   // ── Large gesture label (shadow pass + colour pass) ───────────────────────
    // Scale font so the label always fits within the display width
    double fontScale = 4.0;
    int thickness = 5;
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(gesture, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);

    // Shrink until it fits within 90% of the display width
    while (textSize.width > display.cols * 0.90 && fontScale > 0.5)
    {
        fontScale -= 0.1;
        textSize = cv::getTextSize(gesture, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);
    }

    const cv::Point labelPos{ 40, 120 };
    cv::putText(display, gesture, labelPos,
        cv::FONT_HERSHEY_SIMPLEX, fontScale, { 0, 0, 0 }, thickness * 2); // shadow
    cv::putText(display, gesture, labelPos,
        cv::FONT_HERSHEY_SIMPLEX, fontScale, { 50, 200, 50 }, thickness);  // foreground

    // ── Small debug line ──────────────────────────────────────────────────────
    //std::string info = "Fingers: " + std::to_string(f.fingerCount)
    //    + "  Solidity: " + std::to_string(static_cast<int>(f.solidity * 100)) + "%"
    //    + "  AR: " + std::to_string(f.aspectRatio).substr(0, 4);

    //cv::putText(display, info, { 20, 90 },
    //    cv::FONT_HERSHEY_SIMPLEX, 0.6, { 0, 0, 0 }, 3);      // shadow
    //cv::putText(display, info, { 20, 90 },
    //    cv::FONT_HERSHEY_SIMPLEX, 0.6, { 200, 200, 200 }, 1); // foreground
}
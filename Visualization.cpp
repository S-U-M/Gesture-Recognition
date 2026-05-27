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
    const cv::Point labelPos{ 40, 120 };

    cv::putText(display, gesture, labelPos,
        cv::FONT_HERSHEY_SIMPLEX, 4.0, { 0, 0, 0 }, 10);   // shadow
    cv::putText(display, gesture, labelPos,
        cv::FONT_HERSHEY_SIMPLEX, 4.0, { 50, 200, 50 }, 5); // foreground

    // ── Small debug line ──────────────────────────────────────────────────────
    std::string info = "Fingers: " + std::to_string(f.fingerCount)
        + "  Solidity: " + std::to_string(static_cast<int>(f.solidity * 100)) + "%"
        + "  AR: " + std::to_string(f.aspectRatio).substr(0, 4);

    cv::putText(display, info, { 20, 90 },
        cv::FONT_HERSHEY_SIMPLEX, 0.6, { 0, 0, 0 }, 3);      // shadow
    cv::putText(display, info, { 20, 90 },
        cv::FONT_HERSHEY_SIMPLEX, 0.6, { 200, 200, 200 }, 1); // foreground
}
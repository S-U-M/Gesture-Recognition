#include "Visualization.h"
#include <string>

/**
 * Adds visual feedback to the recognition output image.
 *
 * Preconditions:
 * display is a valid BGR image. f contains the features returned by
 * analyzeHand(), and gesture contains the label returned by classifyGesture().
 *
 * Postconditions:
 * The display image is modified in place. If no hand is valid, only a warning
 * label is drawn.
 */
void drawResults(cv::Mat& display, const HandFeatures& f, const std::string& gesture)
{
    // If hand analysis failed, show a clear message and avoid drawing invalid
    // contour or hull data.
    if (!f.valid) {
        cv::putText(display, "No hand detected", { 20, 40 },
            cv::FONT_HERSHEY_SIMPLEX, 1.2, { 0, 0, 200 }, 2);
        return;
    }

    // Draw the selected hand contour so the user can see which blob was used by
    // the recognition pipeline.
    std::vector<std::vector<cv::Point>> contours = { f.contour };
    cv::drawContours(display, contours, 0, { 0, 200, 0 }, 2);

    // Draw the convex hull to show the simplified outer hand boundary used when
    // computing solidity and convexity defects.
    std::vector<std::vector<cv::Point>> hulls = { f.hull };
    cv::drawContours(display, hulls, 0, { 255, 100, 0 }, 2);

    // Draw the bounding rectangle used to compute aspect ratio and area ratio.
    cv::rectangle(display, f.boundingBox, { 100, 100, 255 }, 2);

    // Start with a large label and shrink it until it fits the output window.
    // This prevents long labels from running off the edge of smaller images.
    double fontScale = 4.0;
    int thickness = 5;
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(
        gesture,
        cv::FONT_HERSHEY_SIMPLEX,
        fontScale,
        thickness,
        &baseline
    );

    while (textSize.width > display.cols * 0.90 && fontScale > 0.5)
    {
        fontScale -= 0.1;
        textSize = cv::getTextSize(
            gesture,
            cv::FONT_HERSHEY_SIMPLEX,
            fontScale,
            thickness,
            &baseline
        );
    }

    // Draw a thick black shadow first, then draw the colored label on top. This
    // makes the gesture name readable on both light and dark backgrounds.
    const cv::Point labelPos{ 40, 120 };
    cv::putText(display, gesture, labelPos,
        cv::FONT_HERSHEY_SIMPLEX, fontScale, { 0, 0, 0 }, thickness * 2);
    cv::putText(display, gesture, labelPos,
        cv::FONT_HERSHEY_SIMPLEX, fontScale, { 50, 200, 50 }, thickness);

    // Optional debug overlay for feature values. This is intentionally left
    // disabled for cleaner result screenshots, but it can be re-enabled when
    // tuning thresholds or explaining the classifier in the write-up.
    // std::string info = "Fingers: " + std::to_string(f.fingerCount)
    //     + "  Solidity: " + std::to_string(static_cast<int>(f.solidity * 100)) + "%"
    //     + "  AR: " + std::to_string(f.aspectRatio).substr(0, 4);

    // cv::putText(display, info, { 20, 90 },
    //     cv::FONT_HERSHEY_SIMPLEX, 0.6, { 0, 0, 0 }, 3);
    // cv::putText(display, info, { 20, 90 },
    //     cv::FONT_HERSHEY_SIMPLEX, 0.6, { 200, 200, 200 }, 1);
}
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

#include "SkinSegmentation.h"
#include "Constants.h"

cv::Mat segmentSkin(const cv::Mat& bgr)
{
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    // Combine two hue ranges so we cover the red-wraparound for darker tones
    cv::Mat mask1, mask2;
    cv::inRange(hsv, SKIN_LOW, SKIN_HIGH, mask1);
    cv::inRange(hsv, SKIN_LOW2, SKIN_HIGH2, mask2);
    cv::Mat mask = mask1 | mask2;

    // Morphological cleanup: two erosion passes remove isolated noise,
    // four dilation passes re-expand and fill small interior holes
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, { 5, 5 });
    cv::erode(mask, mask, kernel, { -1, -1 }, 2);
    cv::dilate(mask, mask, kernel, { -1, -1 }, 4);

    // Slight blur then re-threshold to smooth jagged contour edges
    cv::GaussianBlur(mask, mask, { 5, 5 }, 0);
    cv::threshold(mask, mask, 127, 255, cv::THRESH_BINARY);

    return mask;
}
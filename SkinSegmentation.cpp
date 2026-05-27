#include "SkinSegmentation.h"
#include "Constants.h"

cv::Mat segmentSkin(const cv::Mat& bgr)
{
    cv::Mat blurred;
    cv::GaussianBlur(bgr, blurred, cv::Size(5, 5), 0);

    cv::Mat hsv;
    cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);

    cv::Mat hsvMask1, hsvMask2;
    cv::inRange(hsv, SKIN_LOW, SKIN_HIGH, hsvMask1);
    cv::inRange(hsv, SKIN_LOW2, SKIN_HIGH2, hsvMask2);
    cv::Mat hsvMask = hsvMask1 | hsvMask2;

    cv::Mat ycrcb;
    cv::cvtColor(blurred, ycrcb, cv::COLOR_BGR2YCrCb);

    cv::Mat ycrcbMask;
    cv::inRange(ycrcb, SKIN_YCRCB_LOW, SKIN_YCRCB_HIGH, ycrcbMask);

    // Use OR for robustness across lighting/cameras. Cleanup below removes most noise.
    cv::Mat mask = hsvMask | ycrcbMask;

    cv::Mat kernel3 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::Mat kernel7 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7));

    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel3, cv::Point(-1, -1), 1);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel7, cv::Point(-1, -1), 2);
    cv::dilate(mask, mask, kernel3, cv::Point(-1, -1), 1);

    cv::GaussianBlur(mask, mask, cv::Size(5, 5), 0);
    cv::threshold(mask, mask, 127, 255, cv::THRESH_BINARY);

    return mask;
}

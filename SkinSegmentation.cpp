#include "SkinSegmentation.h"
#include "Constants.h"

/**
 * Converts an input BGR frame into a cleaned binary skin mask.
 *
 * Preconditions:
 * bgr is a valid non-empty BGR image.
 *
 * Postconditions:
 * The input image is unchanged. The returned mask is a single-channel image
 * where skin-like regions are white and other regions are black.
 */
    cv::Mat segmentSkin(const cv::Mat& bgr)
{
    // Convert from BGR to HSV because hue and saturation make skin-color
    // thresholding more stable than raw blue, green, and red channel values.
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    // Apply two HSV ranges. The second range handles hue wraparound near red,
    // which can appear in some skin tones and lighting conditions.
    cv::Mat mask1, mask2;
    cv::inRange(hsv, SKIN_LOW, SKIN_HIGH, mask1);
    cv::inRange(hsv, SKIN_LOW2, SKIN_HIGH2, mask2);
    cv::Mat mask = mask1 | mask2;

    // Remove isolated noise and reconnect small gaps in the detected hand area.
    // Erosion removes tiny white specks, while dilation rebuilds the main blob
    // and fills small holes inside the hand region.
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, { 5, 5 });
    cv::erode(mask, mask, kernel, { -1, -1 }, 2);
    cv::dilate(mask, mask, kernel, { -1, -1 }, 4);

    // Smooth jagged edges, then force the image back to a clean binary mask so
    // contour detection receives only 0 and 255 pixel values.
    cv::GaussianBlur(mask, mask, { 5, 5 }, 0);
    cv::threshold(mask, mask, 127, 255, cv::THRESH_BINARY);

    return mask;
}

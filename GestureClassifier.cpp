#include "GestureClassifier.h"

std::string classifyGesture(const HandFeatures& f)
{
    if (!f.valid) {
        return "No hand detected";
    }

    const int fc = f.fingerCount;
    const double ar = f.aspectRatio;
    const double sol = f.solidity;
    const double extent = f.areaRatio;

    if (fc <= 0) {
        return "Fist";
    }

    if (fc == 1) {
        // Vertical, compact one-finger shape is usually thumbs-up.
        if (ar < 0.90 && sol > 0.68) {
            return "Thumbs Up";
        }

        // Horizontal or less compact one-finger shape is usually pointing.
        return "Pointing";
    }

    if (fc == 2 || fc == 3) {
        // Peace signs usually have clear valleys and lower solidity than a fist.
        if (sol < 0.90 || extent < 0.62) {
            return "Peace Sign";
        }
    }

    if (fc == 4) {
        if (sol < 0.82) {
            return "Okay Sign";
        }
        return "Open Palm";
    }

    if (fc >= 5) {
        if (ar > 0.85) {
            return "Stop Gesture";
        }
        return "Open Palm";
    }

    return "Unknown";
}

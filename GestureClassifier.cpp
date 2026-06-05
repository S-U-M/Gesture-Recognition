#include "GestureClassifier.h"

/**
 * Classifies a detected hand using simple threshold-based rules.
 *
 * Preconditions:
 * f is the feature set produced by analyzeHand(). The function also accepts an
 * invalid HandFeatures object and handles it safely.
 *
 * Postconditions:
 * f is unchanged. A label string is returned for display and console output.
 */
std::string classifyGesture(const HandFeatures& f)
{
    // If no usable contour was found, stop classification immediately. This
    // prevents the classifier from interpreting default feature values as a hand.
    if (!f.valid)
        return "No hand detected";

    // Store the most important features in shorter local variables so the rule
    // conditions are easier to read and tune.
    const int    fc = f.fingerCount;
    const double ar = f.aspectRatio;
    const double sol = f.solidity;

    // A count of zero means the contour did not produce reliable raised-finger
    // evidence. In this project, that case is treated as Unknown instead of Fist
    // to avoid overclaiming when the hand shape is unclear.
    if (fc <= 0)
        return "Unknown";

    // One raised part may represent either a pointing finger or a thumb. A
    // thumbs-up contour is usually compact enough to have moderate solidity, so
    // solidity and aspect ratio are used together to separate the two cases.
    if (fc == 1)
    {
        if (ar > 0.60 && sol > 0.60)
            return "Thumbs Up";
        return "Pointing";
    }

    // Peace signs often produce two or three counted finger regions depending
    // on which convexity defects survive filtering. Lower solidity supports the
    // Peace Sign decision because the spread fingers create visible gaps.
    if (fc == 2 || fc == 3)
    {
        if (sol < 0.85)
            return "Peace Sign";
    }

    // An Okay Sign can create several contour gaps around the circular finger
    // shape. Lower solidity helps distinguish it from a compact open hand.
    if (fc == 4 && sol < 0.80)
        return "Okay Sign";

    // Five or more raised regions indicate an open palm facing the camera. The
    // current project labels this as Stop Gesture for the test image set.
    if (fc >= 5)
    {
        if (ar > 0.90)
            return "Stop Gesture";
        return "Stop Gesture";
    }

    // Any feature combination not covered above is left unclassified.
    return "Unknown";
}

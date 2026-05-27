#include "GestureClassifier.h"

std::string classifyGesture(const HandFeatures& f)
{
    if (!f.valid)
        return "No hand detected";

    const int    fc = f.fingerCount;
    const double ar = f.aspectRatio;   // width / height
    const double sol = f.solidity;

    // ── 0 fingers → closed fist ───────────────────────────────────────────────
    if (fc <= 0)
        return "Fist";

    // ── 1 finger or thumb ─────────────────────────────────────────────────────
    // Thumbs Up: narrow + tall (ar < 0.85) AND solid hull (sol > 0.75).
    //   - ar threshold raised from 0.75 to 0.85: catches thumbs that sit
    //     slightly off-axis without admitting wide blobs.
    //   - solidity gate added: an isolated thumb+palm is compact; a pointing
    //     finger with gap artifacts has lower solidity.
    if (fc == 1)
    {
        if (ar > 0.60 && sol > 0.60)
            return "Thumbs Up";
        return "Pointing";
    }

    // ── 2 fingers ─────────────────────────────────────────────────────────────
    if (fc == 2 || fc == 3)   // 2 gaps + 1 = 3, but peace has 2 raised fingers
    {
        // Peace sign has a visible gap between index+middle and the curled fingers,
        // giving 2 defects → new formula → fc == 3; but also guard fc==2 in case
        // one defect is filtered out
        if (sol < 0.85)
            return "Peace Sign";
    }
    if (fc == 4 && sol < 0.80)
        return "Okay Sign";

    // ── 4–5 fingers ───────────────────────────────────────────────────────────
    if (fc >= 5)
    {
        // Wide spread hand → Stop / flat palm facing camera
        if (ar > 0.90)
            return "Stop Gesture";
        return "Open Palm";
    }

    return "Unknown";
}
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
        if (ar < 0.85 && sol > 0.75)
            return "Thumbs Up";
        return "Pointing";
    }

    // ── 2 fingers ─────────────────────────────────────────────────────────────
    if (fc == 2)
        return "Peace Sign";

    // ── 3 fingers — tentative Okay (ring + index + middle up, circle formed) ──
    if (fc == 3 && sol < 0.80)
        return "Okay Sign";

    // ── 4–5 fingers ───────────────────────────────────────────────────────────
    if (fc >= 4)
    {
        // Wide spread hand → Stop / flat palm facing camera
        if (ar > 0.90)
            return "Stop Gesture";
        return "Open Palm";
    }

    return "Unknown";
}
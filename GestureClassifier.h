#pragma once
#include "HandAnalysis.h"
#include <string>

/**
 * @brief Maps HandFeatures to a gesture label using rule-based logic.
 *
 * Decision table
 * ──────────────────────────────────────────────────────────────────
 * fingerCount | aspectRatio (ar) | solidity (sol) | Result
 * ────────────┼─────────────────┼────────────────┼─────────────────
 *      0      |       any       |      any       | Fist
 *      1      |    ar < 0.85    |   sol > 0.75   | Thumbs Up
 *      1      |    otherwise    |      any       | Pointing
 *      2      |       any       |      any       | Peace Sign
 *      3      |       any       |   sol < 0.80   | Okay Sign
 *    4 or 5   |    ar > 0.90    |      any       | Stop Gesture
 *    4 or 5   |    otherwise    |      any       | Open Palm
 *    other    |       any       |      any       | Unknown
 * ──────────────────────────────────────────────────────────────────
 *
 * Thumbs Up rationale
 * -------------------
 * A thumbs-up blob is narrow and tall  →  ar (width/height) is small.
 * The palm and thumb together form a fairly solid hull  →  sol is high.
 * The tightened ar threshold (0.85, was 0.75) and the added solidity
 * gate (sol > 0.75) make the classifier much less likely to confuse a
 * wide-framed thumb with a pointing finger.
 *
 * @param f  Features returned by analyzeHand().
 * @return   Human-readable gesture name, or "No hand detected".
 */
std::string classifyGesture(const HandFeatures& f);
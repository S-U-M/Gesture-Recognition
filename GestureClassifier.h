#pragma once
#include "HandAnalysis.h"
#include <string>

/**
 * File: GestureClassifier.h
 * Project: Hand Gesture Recognition System
 * Author: Taylor Kang and Sumana Yeluripati
 *
 * Purpose:
 * Declares the rule-based classifier that converts extracted hand features into
 * a human-readable gesture label. This module separates classification logic
 * from image processing, which keeps the project modular, easier to test, and
 * easier to tune.
 *
 * Assumptions:
 * The HandFeatures object was created by analyzeHand(). Finger count, aspect
 * ratio, and solidity are expected to be meaningful when f.valid is true.
 * If f.valid is false, the classifier does not use the feature values.
 */

 /**
  * Maps hand geometry features to a gesture label using rule-based logic.
  *
  * Preconditions:
  * f should be the output of analyzeHand(). If f.valid is false, the classifier
  * safely returns "No hand detected" instead of using invalid geometry values.
  *
  * Postconditions:
  * The input HandFeatures object is not modified.
  *
  * Decision table:
  *
  * fingerCount | aspectRatio (ar) | solidity (sol) | Result
  * ------------|------------------|----------------|-----------------
  * invalid     | any              | any            | No hand detected
  * 0 or less   | any              | any            | Unknown
  * 1           | ar > 0.60        | sol > 0.60     | Thumbs Up
  * 1           | otherwise        | any            | Pointing
  * 2 or 3      | any              | sol < 0.85     | Peace Sign
  * 4           | any              | sol < 0.80     | Okay Sign
  * 5 or more   | any              | any            | Stop Gesture
  * other       | any              | any            | Unknown
  *
  * Thumbs Up rationale:
  * A thumbs-up gesture usually produces a compact hand contour with one dominant
  * raised shape. The aspect ratio check helps distinguish a thumb-like shape
  * from other one-finger gestures, while the solidity check confirms that the
  * palm and thumb form a relatively filled convex hull. This reduces confusion
  * between Thumbs Up and Pointing when the contour has noise or small gaps.
  *
  * Peace Sign rationale:
  * A peace sign usually creates visible gaps between raised fingers, lowering
  * solidity compared with a closed or compact hand shape. Therefore, the
  * classifier checks for two or three detected finger-related features combined
  * with lower solidity.
  *
  * Parameters:
  * f: HandFeatures structure returned by analyzeHand().
  *
  * Returns:
  * Human-readable gesture label, such as "Thumbs Up", "Pointing",
  * "Peace Sign", "Okay Sign", "Stop Gesture", "Unknown", or
  * "No hand detected".
  */
std::string classifyGesture(const HandFeatures& f);
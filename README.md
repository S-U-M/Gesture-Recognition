# Hand Gesture Recognition

A rule-based hand gesture recognizer built with OpenCV 4.x and C++17. It segments skin tone from a BGR image, extracts convex hull features from the largest hand contour, and classifies the result into one of six gestures using a lightweight decision tree — no ML runtime required.

## Supported gestures

| Gesture | Fingers detected |
|---|---|
| Thumbs up | 1 (narrow + solid blob) |
| Pointing | 1 (other) |
| Peace sign | 2 |
| Okay sign | 3 (low solidity) |
| Stop gesture | 4–5 (wide aspect ratio) |
| Open palm | 4–5 (upright) |

## Project structure

```
gesture_recognition/
├── constants.h              — all tuneable thresholds
├── skin_segmentation.h/cpp  — segmentSkin()
├── hand_analysis.h/cpp      — HandFeatures struct + analyzeHand()
├── gesture_classifier.h/cpp — classifyGesture()
├── visualization.h/cpp      — drawResults()
├── main.cpp                 — processFrame() + main()
```

## Build

```bash
cmake -B build -S .
cmake --build build
```

Requires OpenCV 4.x. Tested on Visual Studio 2022 (Windows) and GCC 12 (Linux).

## Usage

```bash
# Single image
./gesture_recognition thumbs_up.jpg

# Batch
./gesture_recognition img1.jpg img2.jpg img3.jpg
```

Press any key to advance through images. A second debug window showing the binary skin mask opens automatically — set `SHOW_MASK = false` in `main.cpp` to disable it.

## Pipeline

The recognizer runs six stages on every frame:

1. **Skin segmentation** — converts BGR → HSV and applies two `inRange()` passes (one for lighter skin tones, one for the red wraparound at the high hue end). The mask is cleaned with two erosion passes and four dilation passes, then Gaussian-blurred and re-thresholded to smooth jagged edges.

2. **Contour detection** — finds all external contours in the mask and keeps only the largest by area. Blobs below `MIN_HAND_AREA` (8 000 px²) are discarded as background noise.

3. **Convexity defect analysis** — computes the convex hull and runs `convexityDefects()`. Three filters are applied before a defect is counted as a finger gap: depth ≥ 40 px, opening angle < 90°, and the far-point must sit in the top 75% of the bounding box (wrist-exclusion zone). The finger count equals the number of surviving gaps plus one, but only when at least one gap was found — zero gaps correctly maps to a fist rather than being promoted to one finger.

4. **HandFeatures** — the contour, hull, bounding box, finger count, aspect ratio (`width / height`), and solidity (`contour area / hull area`) are bundled into a struct and passed downstream.

5. **Gesture classification** — a rule-based decision tree checks `fingerCount` first, then uses `aspectRatio` and `solidity` as tie-breakers where needed (see diagram below).

6. **Visualisation** — the contour, convex hull, and bounding rect are overlaid on the frame alongside a large gesture label and a debug line showing finger count, solidity, and aspect ratio.

### Workflow diagram

<img width="1440" height="1720" alt="image" src="https://github.com/user-attachments/assets/99a24ace-afbf-41e2-afd8-176f3a3e19bf" />

## Classifier logic

`classifyGesture()` is a pure decision tree — no trained weights, no thresholds learned from data. `fingerCount` is the primary discriminator; `aspectRatio` and `solidity` only come into play at two branches:

- **Thumbs up vs pointing** — both `ar < 0.85` (narrow/tall blob) and `sol > 0.75` (compact hull) must hold simultaneously. A wide or jagged single-finger blob falls through to Pointing.
- **Okay sign vs unknown** — `sol < 0.80` captures the hull concavity created by the thumb-index circle. High solidity (three fingers spread straight) returns Unknown, which is the honest answer for a gesture the rule set doesn't cover.

### Classifier decision tree

<img width="1440" height="1400" alt="image" src="https://github.com/user-attachments/assets/73822405-fbfd-4b58-8b67-c6952d04f79e" />

## Tuning

All thresholds live in `constants.hpp`. The most impactful ones:

| Constant | Default | Effect |
|---|---|---|
| `MIN_HAND_AREA` | 8 000 px² | Raise if background blobs are being tracked |
| `MIN_DEFECT_DEPTH` | 40 px | Lower to count shallower finger gaps; raise to suppress noise |
| `MAX_DEFECT_ANGLE_DEG` | 90° | Raise to count wider finger spreads |
| `WRIST_CUTOFF_FRACTION` | 0.75 | Lower to exclude more of the forearm from defect counting |
| `SKIN_LOW / SKIN_HIGH` | H 0–20 | Adjust hue range for different lighting conditions |

## Known limitations

- Skin segmentation is HSV-range based, so it is sensitive to lighting colour temperature. Fluorescent and mixed lighting may require adjusting `SKIN_LOW`/`SKIN_HIGH`.
- The classifier has no concept of hand orientation — a sideways thumbs-up will likely misclassify.
- Okay sign detection is approximate; a proper implementation would benefit from fingertip landmark detection.

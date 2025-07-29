//
// Created by Mateusz on 18.06.2025.
//

#include "../lib/aiRecognition.h"

aiRecognition::aiRecognition() {
}

aiRecognition::~aiRecognition() {
}

bool aiRecognition::startRecognition(int originalImageSizeX, int originalImageSizeY, int selectedImageSizeX, int selectedImageSizeY) {
    spdlog::info("aiRecognition: Starting recognition with original image size {}x{} and selected image size {}x{}",
        originalImageSizeX, originalImageSizeY, selectedImageSizeX, selectedImageSizeY);

    if (!generateMetaData()) {
        spdlog::info("aiRecognition: Failed to Generate Meta Data");
        return false;
    }
    spdlog::info("aiRecognition: Generated Meta Data");

    if (!recognizePatterns()) {
        spdlog::info("aiRecognition: Failed to Recognize patterns");
        return false;
    }
    spdlog::info("aiRecognition: Recognized patterns");

    return true;
}

bool aiRecognition::generateMetaData() {
    //TODO - get the size of every decomposed image
    return true;
}

bool aiRecognition::recognizePatterns() {
    return true;
}

bool aiRecognition::recognizePatternSingleImage() {
    return true;
}

float aiRecognition::isStraightVerticalLine() {
    return 0.0f;
}

float aiRecognition::isStraightHorizontalLine() {
    return 0.0f;
}

float aiRecognition::isStraightDiagonalLine() {
    return 0.0f;
}

float aiRecognition::isAlternatingVerticalLine() {
    return 0.0f;
}

float aiRecognition::isAlternatingHorizontalLine() {
    return 0.0f;
}

float aiRecognition::isAlternatingDiagonalLine() {
    return 0.0f;
}

float aiRecognition::isCircle() {
    return 0.0f;
}

float aiRecognition::isGridPattern() {
    return 0.0f;
}

float aiRecognition::isAlternatingCircumferencePattern() {
    return 0.0f;
}

float aiRecognition::isAlternatingGridPattern() {
    return 0.0f;
}

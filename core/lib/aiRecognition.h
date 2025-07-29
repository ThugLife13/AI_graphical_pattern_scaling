//
// Created by Mateusz on 18.06.2025.
//

#ifndef AIRECOGNITION_H
#define AIRECOGNITION_H

#include "core.h"

enum patternType {
    STRAIGHT_VERTICAL_LINE = 0,
    STRAIGHT_HORIZONTAL_LINE,
    STRAIGHT_DIAGONAL_LINE,
    ALTERNATING_VERTICAL_LINE,
    ALTERNATING_HORIZONTAL_LINE,
    ALTERNATING_DIAGONAL_LINE,
    CIRCLE,
    GRID_PATTERN,
    CIRCUMFERENCE_PATTERN,
    ALTERNATING_CIRCUMFERENCE_PATTERN,
    ALTERNATING_GRID_PATTERN,
    RANDOM_PATTERN
};

class aiRecognition {
public:
    aiRecognition();
    ~aiRecognition();

    bool startRecognition(int originalImageSizeX, int originalImageSizeY, int selectedImageSizeX, int selectedImageSizeY);

private:
    //generate meta of whole picture and decomposed elements
    bool generateMetaData();
    //TODO - get the size and pos of each decomposed (based from original image)

    //recognize all patterns
    bool recognizePatterns();

    //recognize patterns based on id - selecting the highest accuracy from "is" functions
    bool recognizePatternSingleImage();

    //returns float number from 0.0 to 1.0 - accuracy compared to the perfect pattern
    float isStraightVerticalLine();
    float isStraightHorizontalLine();
    float isStraightDiagonalLine();
    float isAlternatingVerticalLine();
    float isAlternatingHorizontalLine();
    float isAlternatingDiagonalLine();
    float isCircle();
    float isGridPattern();
    float isAlternatingCircumferencePattern();
    float isAlternatingGridPattern();

    //random pattern (defining the spread of items on image)

};



#endif //AIRECOGNITION_H

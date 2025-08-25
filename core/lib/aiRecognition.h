#ifndef AIRECOGNITION_H
#define AIRECOGNITION_H

#include "core.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

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

struct PatternResult {
    patternType type;
    float score;
    std::map<std::string, float> details;
};

class aiRecognition {
public:
    aiRecognition();
    ~aiRecognition();

    bool startRecognition();

private:
    //generate meta of whole picture and decomposed elements
    bool generateMetaData();

    //recognize all patterns
    bool recognizePatterns();

    //recognize patterns based on id - selecting the highest accuracy from "is" functions
    PatternResult recognizePatternSingleImage(const std::string& jsonFilePath);

    //returns float number from 0.0 to 1.0 - accuracy compared to the perfect pattern
    float isStraightVerticalLine(const nlohmann::json& matches, std::map<std::string, float>& details);
    float isStraightHorizontalLine(const nlohmann::json& matches, std::map<std::string, float>& details);
    float isStraightDiagonalLine(const nlohmann::json& matches, std::map<std::string, float>& details);
    float isAlternatingVerticalLine(const nlohmann::json& matches, std::map<std::string, float>& details);
    float isAlternatingHorizontalLine(const nlohmann::json& matches, std::map<std::string, float>& details);
    float isAlternatingDiagonalLine(const nlohmann::json& matches, std::map<std::string, float>& details);
    float isCircle(const nlohmann::json& matches, std::map<std::string, float>& details);
    float isGridPattern(const nlohmann::json& matches, std::map<std::string, float>& details);
    float isAlternatingCircumferencePattern(const nlohmann::json& matches, std::map<std::string, float>& details);
    float isAlternatingGridPattern(const nlohmann::json& matches, std::map<std::string, float>& details);

    std::vector<std::tuple<std::string, int, int, int, int>> decomposedPositions;

    //random pattern (defining the spread of items on image)

};

#endif //AIRECOGNITION_H
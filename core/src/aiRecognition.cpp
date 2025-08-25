#include "../lib/aiRecognition.h"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <numeric>

// Tolerance for pattern matching (10 pixels as requested)
const float PATTERN_TOLERANCE = 10.0f;

aiRecognition::aiRecognition() {
}

aiRecognition::~aiRecognition() {
}

bool aiRecognition::startRecognition() {
    spdlog::info("aiRecognition: Starting recognition");

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
    // Implementation would go here
    return true;
}

bool aiRecognition::recognizePatterns() {
    std::string directoryPath = "../tmp/jsons/newAccuracy";
    std::vector<std::string> jsonFiles;

    // Get all JSON files in the directory
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".json") {
            jsonFiles.push_back(entry.path().string());
        }
    }

    // Process each JSON file
    for (const auto& jsonFile : jsonFiles) {
        PatternResult result = recognizePatternSingleImage(jsonFile);

        // Save results to a new JSON file
        nlohmann::json resultJson;
        resultJson["patternType"] = result.type;
        resultJson["score"] = result.score;
        resultJson["details"] = result.details;

        spdlog::info("aiRecognition: Saving recognized pattern");
        std::string outputPath = "../tmp/patterns/meta_" + std::filesystem::path(jsonFile).filename().string();
        std::ofstream outputFile(outputPath);
        outputFile << resultJson.dump(4);
        outputFile.close();
    }

    return true;
}

PatternResult aiRecognition::recognizePatternSingleImage(const std::string& jsonFilePath) {
    std::ifstream file(jsonFilePath);
    nlohmann::json jsonData;
    file >> jsonData;

    std::vector<PatternResult> results;
    std::map<std::string, float> details;

    // Test all pattern types
    results.push_back({STRAIGHT_VERTICAL_LINE, isStraightVerticalLine(jsonData, details), details});
    details.clear();
    results.push_back({STRAIGHT_HORIZONTAL_LINE, isStraightHorizontalLine(jsonData, details), details});
    details.clear();
    results.push_back({STRAIGHT_DIAGONAL_LINE, isStraightDiagonalLine(jsonData, details), details});
    details.clear();
    results.push_back({ALTERNATING_VERTICAL_LINE, isAlternatingVerticalLine(jsonData, details), details});
    details.clear();
    results.push_back({ALTERNATING_HORIZONTAL_LINE, isAlternatingHorizontalLine(jsonData, details), details});
    details.clear();
    results.push_back({ALTERNATING_DIAGONAL_LINE, isAlternatingDiagonalLine(jsonData, details), details});
    details.clear();
    results.push_back({CIRCLE, isCircle(jsonData, details), details});
    details.clear();
    results.push_back({GRID_PATTERN, isGridPattern(jsonData, details), details});
    details.clear();
    results.push_back({ALTERNATING_CIRCUMFERENCE_PATTERN, isAlternatingCircumferencePattern(jsonData, details), details});
    details.clear();
    results.push_back({ALTERNATING_GRID_PATTERN, isAlternatingGridPattern(jsonData, details), details});

    // Find the pattern with highest score
    auto bestResult = std::max_element(results.begin(), results.end(),
        [](const PatternResult& a, const PatternResult& b) {
            return a.score < b.score;
        });

    return *bestResult;
}

float aiRecognition::isStraightVerticalLine(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.empty()) return 0.0f;

    std::vector<float> xPositions;
    std::vector<float> yPositions;
    std::vector<float> accuracies;

    for (const auto& match : matches) {
        float centerX = match["Pos.X"].get<float>() + match["Pos.Width"].get<float>() / 2.0f;
        float centerY = match["Pos.Y"].get<float>() + match["Pos.Height"].get<float>() / 2.0f;
        xPositions.push_back(centerX);
        yPositions.push_back(centerY);
        accuracies.push_back(match["Pos.Accuracy"].get<float>());
    }

    // Calculate weighted mean of X positions
    float sumX = 0.0f;
    float totalWeight = 0.0f;
    for (size_t i = 0; i < xPositions.size(); i++) {
        sumX += xPositions[i] * accuracies[i];
        totalWeight += accuracies[i];
    }
    float meanX = sumX / totalWeight;

    // Calculate variance from the mean
    float variance = 0.0f;
    for (size_t i = 0; i < xPositions.size(); i++) {
        variance += accuracies[i] * std::pow(xPositions[i] - meanX, 2);
    }
    variance /= totalWeight;

    // Check if Y positions are in order
    float orderScore = 1.0f;
    std::vector<size_t> yOrder(yPositions.size());
    std::iota(yOrder.begin(), yOrder.end(), 0);
    std::sort(yOrder.begin(), yOrder.end(), [&yPositions](size_t i, size_t j) {
        return yPositions[i] < yPositions[j];
    });

    for (size_t i = 1; i < yOrder.size(); i++) {
        if (yOrder[i] < yOrder[i-1]) {
            orderScore -= 0.1f;
        }
    }
    orderScore = std::max(0.0f, orderScore);

    // Calculate final score
    float stdDev = std::sqrt(variance);
    float deviationScore = std::max(0.0f, 1.0f - stdDev / PATTERN_TOLERANCE);
    float score = (deviationScore * 0.7f) + (orderScore * 0.3f);

    // Store details
    details["meanX"] = meanX;
    details["stdDev"] = stdDev;
    details["pointCount"] = xPositions.size();
    details["deviationScore"] = deviationScore;
    details["orderScore"] = orderScore;

    return score;
}

float aiRecognition::isStraightHorizontalLine(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.empty()) return 0.0f;

    std::vector<float> xPositions;
    std::vector<float> yPositions;
    std::vector<float> accuracies;

    for (const auto& match : matches) {
        float centerX = match["Pos.X"].get<float>() + match["Pos.Width"].get<float>() / 2.0f;
        float centerY = match["Pos.Y"].get<float>() + match["Pos.Height"].get<float>() / 2.0f;
        xPositions.push_back(centerX);
        yPositions.push_back(centerY);
        accuracies.push_back(match["Pos.Accuracy"].get<float>());
    }

    // Calculate weighted mean of Y positions
    float sumY = 0.0f;
    float totalWeight = 0.0f;
    for (size_t i = 0; i < yPositions.size(); i++) {
        sumY += yPositions[i] * accuracies[i];
        totalWeight += accuracies[i];
    }
    float meanY = sumY / totalWeight;

    // Calculate variance from the mean
    float variance = 0.0f;
    for (size_t i = 0; i < yPositions.size(); i++) {
        variance += accuracies[i] * std::pow(yPositions[i] - meanY, 2);
    }
    variance /= totalWeight;

    // Check if X positions are in order
    float orderScore = 1.0f;
    std::vector<size_t> xOrder(xPositions.size());
    std::iota(xOrder.begin(), xOrder.end(), 0);
    std::sort(xOrder.begin(), xOrder.end(), [&xPositions](size_t i, size_t j) {
        return xPositions[i] < xPositions[j];
    });

    for (size_t i = 1; i < xOrder.size(); i++) {
        if (xOrder[i] < xOrder[i-1]) {
            orderScore -= 0.1f;
        }
    }
    orderScore = std::max(0.0f, orderScore);

    // Calculate final score
    float stdDev = std::sqrt(variance);
    float deviationScore = std::max(0.0f, 1.0f - stdDev / PATTERN_TOLERANCE);
    float score = (deviationScore * 0.7f) + (orderScore * 0.3f);

    // Store details
    details["meanY"] = meanY;
    details["stdDev"] = stdDev;
    details["pointCount"] = xPositions.size();
    details["deviationScore"] = deviationScore;
    details["orderScore"] = orderScore;

    return score;
}

float aiRecognition::isStraightDiagonalLine(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.size() < 2) return 0.0f;

    std::vector<float> xPositions;
    std::vector<float> yPositions;
    std::vector<float> accuracies;

    for (const auto& match : matches) {
        float centerX = match["Pos.X"].get<float>() + match["Pos.Width"].get<float>() / 2.0f;
        float centerY = match["Pos.Y"].get<float>() + match["Pos.Height"].get<float>() / 2.0f;
        xPositions.push_back(centerX);
        yPositions.push_back(centerY);
        accuracies.push_back(match["Pos.Accuracy"].get<float>());
    }

    // Calculate line parameters using linear regression
    float sumX = 0.0f, sumY = 0.0f, sumXY = 0.0f, sumX2 = 0.0f;
    float totalWeight = 0.0f;

    for (size_t i = 0; i < xPositions.size(); i++) {
        float weight = accuracies[i];
        sumX += xPositions[i] * weight;
        sumY += yPositions[i] * weight;
        sumXY += xPositions[i] * yPositions[i] * weight;
        sumX2 += xPositions[i] * xPositions[i] * weight;
        totalWeight += weight;
    }

    float meanX = sumX / totalWeight;
    float meanY = sumY / totalWeight;

    float denominator = sumX2 - totalWeight * meanX * meanX;
    if (denominator == 0) return 0.0f;

    float slope = (sumXY - totalWeight * meanX * meanY) / denominator;
    float intercept = meanY - slope * meanX;

    // Calculate average distance from the line
    float totalDistance = 0.0f;
    for (size_t i = 0; i < xPositions.size(); i++) {
        float expectedY = slope * xPositions[i] + intercept;
        float distance = std::abs(yPositions[i] - expectedY);
        totalDistance += distance * accuracies[i];
    }
    float avgDistance = totalDistance / totalWeight;

    // Calculate score based on distance from ideal line
    float distanceScore = std::max(0.0f, 1.0f - avgDistance / PATTERN_TOLERANCE);

    // Store details
    details["slope"] = slope;
    details["intercept"] = intercept;
    details["avgDistance"] = avgDistance;
    details["pointCount"] = xPositions.size();

    return distanceScore;
}

float aiRecognition::isAlternatingVerticalLine(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.size() < 4) return 0.0f;

    // First check if it's a straight vertical line
    float straightScore = isStraightVerticalLine(matches, details);
    if (straightScore < 0.5f) return 0.0f;

    std::vector<float> xPositions;
    std::vector<float> yPositions;

    for (const auto& match : matches) {
        float centerX = match["Pos.X"].get<float>() + match["Pos.Width"].get<float>() / 2.0f;
        float centerY = match["Pos.Y"].get<float>() + match["Pos.Height"].get<float>() / 2.0f;
        xPositions.push_back(centerX);
        yPositions.push_back(centerY);
    }

    // Check for alternating pattern in colors
    int firstColor = matches[0]["Color.First"].get<int>();
    int secondColor = matches[0]["Color.Second"].get<int>();

    int alternatingErrors = 0;
    for (size_t i = 1; i < matches.size(); i++) {
        int currentFirst = matches[i]["Color.First"].get<int>();
        int currentSecond = matches[i]["Color.Second"].get<int>();

        if (i % 2 == 0) {
            if (currentFirst != firstColor || currentSecond != secondColor) {
                alternatingErrors++;
            }
        } else {
            if (currentFirst != secondColor || currentSecond != firstColor) {
                alternatingErrors++;
            }
        }
    }

    float colorScore = 1.0f - (alternatingErrors / static_cast<float>(matches.size()));

    // Calculate final score
    float score = straightScore * 0.7f + colorScore * 0.3f;

    // Store details
    details["straightScore"] = straightScore;
    details["colorScore"] = colorScore;
    details["alternatingErrors"] = alternatingErrors;

    return score;
}

float aiRecognition::isAlternatingHorizontalLine(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.size() < 4) return 0.0f;

    // First check if it's a straight horizontal line
    float straightScore = isStraightHorizontalLine(matches, details);
    if (straightScore < 0.5f) return 0.0f;

    // Check for alternating pattern in colors
    int firstColor = matches[0]["Color.First"].get<int>();
    int secondColor = matches[0]["Color.Second"].get<int>();

    int alternatingErrors = 0;
    for (size_t i = 1; i < matches.size(); i++) {
        int currentFirst = matches[i]["Color.First"].get<int>();
        int currentSecond = matches[i]["Color.Second"].get<int>();

        if (i % 2 == 0) {
            if (currentFirst != firstColor || currentSecond != secondColor) {
                alternatingErrors++;
            }
        } else {
            if (currentFirst != secondColor || currentSecond != firstColor) {
                alternatingErrors++;
            }
        }
    }

    float colorScore = 1.0f - (alternatingErrors / static_cast<float>(matches.size()));

    // Calculate final score
    float score = straightScore * 0.7f + colorScore * 0.3f;

    // Store details
    details["straightScore"] = straightScore;
    details["colorScore"] = colorScore;
    details["alternatingErrors"] = alternatingErrors;

    return score;
}

float aiRecognition::isAlternatingDiagonalLine(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.size() < 4) return 0.0f;

    // First check if it's a straight diagonal line
    float straightScore = isStraightDiagonalLine(matches, details);
    if (straightScore < 0.5f) return 0.0f;

    // Check for alternating pattern in colors
    int firstColor = matches[0]["Color.First"].get<int>();
    int secondColor = matches[0]["Color.Second"].get<int>();

    int alternatingErrors = 0;
    for (size_t i = 1; i < matches.size(); i++) {
        int currentFirst = matches[i]["Color.First"].get<int>();
        int currentSecond = matches[i]["Color.Second"].get<int>();

        if (i % 2 == 0) {
            if (currentFirst != firstColor || currentSecond != secondColor) {
                alternatingErrors++;
            }
        } else {
            if (currentFirst != secondColor || currentSecond != firstColor) {
                alternatingErrors++;
            }
        }
    }

    float colorScore = 1.0f - (alternatingErrors / static_cast<float>(matches.size()));

    // Calculate final score
    float score = straightScore * 0.7f + colorScore * 0.3f;

    // Store details
    details["straightScore"] = straightScore;
    details["colorScore"] = colorScore;
    details["alternatingErrors"] = alternatingErrors;

    return score;
}

float aiRecognition::isCircle(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.size() < 5) return 0.0f;

    std::vector<float> xPositions;
    std::vector<float> yPositions;
    std::vector<float> accuracies;

    for (const auto& match : matches) {
        float centerX = match["Pos.X"].get<float>() + match["Pos.Width"].get<float>() / 2.0f;
        float centerY = match["Pos.Y"].get<float>() + match["Pos.Height"].get<float>() / 2.0f;
        xPositions.push_back(centerX);
        yPositions.push_back(centerY);
        accuracies.push_back(match["Pos.Accuracy"].get<float>());
    }

    // Calculate centroid
    float sumX = 0.0f, sumY = 0.0f, totalWeight = 0.0f;
    for (size_t i = 0; i < xPositions.size(); i++) {
        sumX += xPositions[i] * accuracies[i];
        sumY += yPositions[i] * accuracies[i];
        totalWeight += accuracies[i];
    }
    float centroidX = sumX / totalWeight;
    float centroidY = sumY / totalWeight;

    // Calculate average radius
    float sumRadius = 0.0f;
    for (size_t i = 0; i < xPositions.size(); i++) {
        float dx = xPositions[i] - centroidX;
        float dy = yPositions[i] - centroidY;
        sumRadius += std::sqrt(dx*dx + dy*dy) * accuracies[i];
    }
    float avgRadius = sumRadius / totalWeight;

    // Calculate radius variance
    float radiusVariance = 0.0f;
    for (size_t i = 0; i < xPositions.size(); i++) {
        float dx = xPositions[i] - centroidX;
        float dy = yPositions[i] - centroidY;
        float radius = std::sqrt(dx*dx + dy*dy);
        radiusVariance += std::pow(radius - avgRadius, 2) * accuracies[i];
    }
    radiusVariance /= totalWeight;

    // Calculate angle distribution uniformity
    std::vector<float> angles;
    for (size_t i = 0; i < xPositions.size(); i++) {
        float dx = xPositions[i] - centroidX;
        float dy = yPositions[i] - centroidY;
        float angle = std::atan2(dy, dx);
        angles.push_back(angle);
    }

    std::sort(angles.begin(), angles.end());

    float maxGap = 0.0f;
    for (size_t i = 1; i < angles.size(); i++) {
        float gap = angles[i] - angles[i-1];
        if (gap > maxGap) maxGap = gap;
    }
    // Check gap between last and first (circular)
    float circularGap = (2 * M_PI) - angles.back() + angles.front();
    if (circularGap > maxGap) maxGap = circularGap;

    float uniformityScore = 1.0f - (maxGap / (2 * M_PI));

    // Calculate final score
    float radiusScore = std::max(0.0f, 1.0f - std::sqrt(radiusVariance) / PATTERN_TOLERANCE);
    float score = (radiusScore * 0.6f) + (uniformityScore * 0.4f);

    // Store details
    details["centroidX"] = centroidX;
    details["centroidY"] = centroidY;
    details["avgRadius"] = avgRadius;
    details["radiusVariance"] = radiusVariance;
    details["uniformityScore"] = uniformityScore;
    details["pointCount"] = xPositions.size();

    return score;
}

float aiRecognition::isGridPattern(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.size() < 9) return 0.0f; // Minimum 3x3 grid

    std::vector<float> xPositions;
    std::vector<float> yPositions;
    std::vector<float> accuracies;

    for (const auto& match : matches) {
        float centerX = match["Pos.X"].get<float>() + match["Pos.Width"].get<float>() / 2.0f;
        float centerY = match["Pos.Y"].get<float>() + match["Pos.Height"].get<float>() / 2.0f;
        xPositions.push_back(centerX);
        yPositions.push_back(centerY);
        accuracies.push_back(match["Pos.Accuracy"].get<float>());
    }

    // Find unique X and Y values (grid lines)
    std::vector<float> uniqueX = xPositions;
    std::sort(uniqueX.begin(), uniqueX.end());
    uniqueX.erase(std::unique(uniqueX.begin(), uniqueX.end()), uniqueX.end());

    std::vector<float> uniqueY = yPositions;
    std::sort(uniqueY.begin(), uniqueY.end());
    uniqueY.erase(std::unique(uniqueY.begin(), uniqueY.end()), uniqueY.end());

    // Check if we have a grid structure
    if (uniqueX.size() < 3 || uniqueY.size() < 3) return 0.0f;

    // Calculate average spacing in X and Y directions
    float sumXSpacing = 0.0f;
    for (size_t i = 1; i < uniqueX.size(); i++) {
        sumXSpacing += uniqueX[i] - uniqueX[i-1];
    }
    float avgXSpacing = sumXSpacing / (uniqueX.size() - 1);

    float sumYSpacing = 0.0f;
    for (size_t i = 1; i < uniqueY.size(); i++) {
        sumYSpacing += uniqueY[i] - uniqueY[i-1];
    }
    float avgYSpacing = sumYSpacing / (uniqueY.size() - 1);

    // Check spacing consistency
    float xSpacingVariance = 0.0f;
    for (size_t i = 1; i < uniqueX.size(); i++) {
        float spacing = uniqueX[i] - uniqueX[i-1];
        xSpacingVariance += std::pow(spacing - avgXSpacing, 2);
    }
    xSpacingVariance /= (uniqueX.size() - 1);

    float ySpacingVariance = 0.0f;
    for (size_t i = 1; i < uniqueY.size(); i++) {
        float spacing = uniqueY[i] - uniqueY[i-1];
        ySpacingVariance += std::pow(spacing - avgYSpacing, 2);
    }
    ySpacingVariance /= (uniqueY.size() - 1);

    // Check if points are at grid intersections
    int gridErrors = 0;
    for (size_t i = 0; i < xPositions.size(); i++) {
        bool foundX = false;
        bool foundY = false;

        for (float x : uniqueX) {
            if (std::abs(xPositions[i] - x) < PATTERN_TOLERANCE) {
                foundX = true;
                break;
            }
        }

        for (float y : uniqueY) {
            if (std::abs(yPositions[i] - y) < PATTERN_TOLERANCE) {
                foundY = true;
                break;
            }
        }

        if (!foundX || !foundY) {
            gridErrors++;
        }
    }

    float gridAccuracy = 1.0f - (gridErrors / static_cast<float>(xPositions.size()));

    // Calculate final score
    float xSpacingScore = std::max(0.0f, 1.0f - std::sqrt(xSpacingVariance) / PATTERN_TOLERANCE);
    float ySpacingScore = std::max(0.0f, 1.0f - std::sqrt(ySpacingVariance) / PATTERN_TOLERANCE);
    float spacingScore = (xSpacingScore + ySpacingScore) / 2.0f;

    float score = (spacingScore * 0.5f) + (gridAccuracy * 0.5f);

    // Store details
    details["gridRows"] = uniqueY.size();
    details["gridColumns"] = uniqueX.size();
    details["avgXSpacing"] = avgXSpacing;
    details["avgYSpacing"] = avgYSpacing;
    details["xSpacingVariance"] = xSpacingVariance;
    details["ySpacingVariance"] = ySpacingVariance;
    details["gridAccuracy"] = gridAccuracy;
    details["gridErrors"] = gridErrors;

    return score;
}

float aiRecognition::isAlternatingCircumferencePattern(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.size() < 8) return 0.0f; // Need enough points for a circle

    // First check if it's a circle
    float circleScore = isCircle(matches, details);
    if (circleScore < 0.5f) return 0.0f;

    // Check for alternating pattern in colors
    int firstColor = matches[0]["Color.First"].get<int>();
    int secondColor = matches[0]["Color.Second"].get<int>();

    int alternatingErrors = 0;
    for (size_t i = 1; i < matches.size(); i++) {
        int currentFirst = matches[i]["Color.First"].get<int>();
        int currentSecond = matches[i]["Color.Second"].get<int>();

        if (i % 2 == 0) {
            if (currentFirst != firstColor || currentSecond != secondColor) {
                alternatingErrors++;
            }
        } else {
            if (currentFirst != secondColor || currentSecond != firstColor) {
                alternatingErrors++;
            }
        }
    }

    float colorScore = 1.0f - (alternatingErrors / static_cast<float>(matches.size()));

    // Calculate final score
    float score = circleScore * 0.7f + colorScore * 0.3f;

    // Store details
    details["circleScore"] = circleScore;
    details["colorScore"] = colorScore;
    details["alternatingErrors"] = alternatingErrors;

    return score;
}

float aiRecognition::isAlternatingGridPattern(const nlohmann::json& matches, std::map<std::string, float>& details) {
    if (matches.size() < 9) return 0.0f; // Minimum 3x3 grid

    // First check if it's a grid
    float gridScore = isGridPattern(matches, details);
    if (gridScore < 0.5f) return 0.0f;

    // Extract all positions
    std::vector<float> xPositions;
    std::vector<float> yPositions;
    std::vector<std::pair<int, int>> colors;

    for (const auto& match : matches) {
        float centerX = match["Pos.X"].get<float>() + match["Pos.Width"].get<float>() / 2.0f;
        float centerY = match["Pos.Y"].get<float>() + match["Pos.Height"].get<float>() / 2.0f;
        int firstColor = match["Color.First"].get<int>();
        int secondColor = match["Color.Second"].get<int>();

        xPositions.push_back(centerX);
        yPositions.push_back(centerY);
        colors.push_back({firstColor, secondColor});
    }

    // Find unique X and Y values (grid lines) with tolerance
    std::vector<float> uniqueX;
    std::vector<float> uniqueY;

    std::sort(xPositions.begin(), xPositions.end());
    for (float x : xPositions) {
        if (uniqueX.empty() || std::abs(x - uniqueX.back()) > PATTERN_TOLERANCE) {
            uniqueX.push_back(x);
        }
    }

    std::sort(yPositions.begin(), yPositions.end());
    for (float y : yPositions) {
        if (uniqueY.empty() || std::abs(y - uniqueY.back()) > PATTERN_TOLERANCE) {
            uniqueY.push_back(y);
        }
    }

    // Assign each point to a grid cell
    std::vector<std::vector<int>> grid(uniqueY.size(), std::vector<int>(uniqueX.size(), -1));
    std::vector<std::vector<std::pair<int, int>>> gridColors(uniqueY.size(),
                                                           std::vector<std::pair<int, int>>(uniqueX.size(), {-1, -1}));

    for (size_t i = 0; i < xPositions.size(); i++) {
        // Find closest grid column
        int col = 0;
        float minDistX = std::abs(xPositions[i] - uniqueX[0]);
        for (size_t j = 1; j < uniqueX.size(); j++) {
            float dist = std::abs(xPositions[i] - uniqueX[j]);
            if (dist < minDistX) {
                minDistX = dist;
                col = j;
            }
        }

        // Find closest grid row
        int row = 0;
        float minDistY = std::abs(yPositions[i] - uniqueY[0]);
        for (size_t j = 1; j < uniqueY.size(); j++) {
            float dist = std::abs(yPositions[i] - uniqueY[j]);
            if (dist < minDistY) {
                minDistY = dist;
                row = j;
            }
        }

        // Assign to grid if within tolerance
        if (minDistX <= PATTERN_TOLERANCE && minDistY <= PATTERN_TOLERANCE) {
            grid[row][col] = i;
            gridColors[row][col] = colors[i];
        }
    }

    // Check for alternating color pattern (checkerboard)
    int colorErrors = 0;
    int totalCells = 0;

    // Determine the expected color pattern for the first cell
    std::pair<int, int> expectedColor1 = gridColors[0][0];
    std::pair<int, int> expectedColor2;

    // Find a neighboring cell to determine the alternating color
    if (gridColors.size() > 1 && gridColors[0].size() > 0) {
        expectedColor2 = gridColors[1][0];
    } else if (gridColors.size() > 0 && gridColors[0].size() > 1) {
        expectedColor2 = gridColors[0][1];
    } else {
        // Not enough cells to determine alternating pattern
        return gridScore * 0.7f; // Return grid score without color pattern
    }

    // Check if the two colors are actually different
    if (expectedColor1 == expectedColor2) {
        // Colors are not alternating
        return gridScore * 0.7f;
    }

    // Verify checkerboard pattern
    for (size_t row = 0; row < grid.size(); row++) {
        for (size_t col = 0; col < grid[row].size(); col++) {
            if (grid[row][col] != -1) { // Cell is occupied
                totalCells++;

                // Determine expected color based on checkerboard pattern
                std::pair<int, int> expectedColor = ((row + col) % 2 == 0) ? expectedColor1 : expectedColor2;

                // Check if actual color matches expected color
                if (gridColors[row][col] != expectedColor) {
                    colorErrors++;
                }
            }
        }
    }

    if (totalCells == 0) return 0.0f;

    float colorScore = 1.0f - (static_cast<float>(colorErrors) / totalCells);

    // Calculate final score
    float score = gridScore * 0.7f + colorScore * 0.3f;

    // Store details
    details["gridScore"] = gridScore;
    details["colorScore"] = colorScore;
    details["colorErrors"] = colorErrors;
    details["totalCells"] = totalCells;
    details["gridRows"] = uniqueY.size();
    details["gridColumns"] = uniqueX.size();

    return score;
}
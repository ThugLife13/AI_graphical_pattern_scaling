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
    //TODO - check if works
    std::vector<nlohmann::json> allPositions;
    std::ifstream posFile("../tmp/metaData/decomposed_positions.json");

    if (posFile.is_open()) {
        std::string line;
        while (std::getline(posFile, line)) {
            if (!line.empty()) {
                allPositions.push_back(nlohmann::json::parse(line));
            }
        }
        posFile.close();
    }

    std::sort(allPositions.begin(), allPositions.end(),
        [](const nlohmann::json& a, const nlohmann::json& b) {
            return a["width"].get<int>() * a["height"].get<int>() >
                   b["width"].get<int>() * b["height"].get<int>();
        });

    nlohmann::json metaData;
    for (size_t i = 0; i < allPositions.size(); ++i) {
        auto& pos = allPositions[i];
        nlohmann::json item;
        item["name"] = pos["name"];
        item["x"] = pos["x"];
        item["y"] = pos["y"];
        item["width"] = pos["width"];
        item["height"] = pos["height"];

        if (i > 0) {
            auto& prev = allPositions[i-1];
            int dx = pos["x"].get<int>() - prev["x"].get<int>();
            int dy = pos["y"].get<int>() - prev["y"].get<int>();
            int dw = pos["width"].get<int>() - prev["width"].get<int>();
            int dh = pos["height"].get<int>() - prev["height"].get<int>();

            item["relative_x"] = dx;
            item["relative_y"] = dy;
            item["relative_width"] = dw;
            item["relative_height"] = dh;
        }

        metaData["elements"].push_back(item);
    }

    std::ofstream outFile("../tmp/metaData/decomposed_meta.json");
    if (!outFile.is_open()) {
        spdlog::error("generateMetaData: Failed to create metadata file");
        return false;
    }

    outFile << metaData.dump(4);
    outFile.close();

    spdlog::info("generateMetaData: Created metadata for {} decomposed images", allPositions.size());
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

#ifndef CORE_H
#define CORE_H

#include "imageProcessing.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <fstream>
#include <cmath>
#include <filesystem>
#include <omp.h>
#include <atomic>
#include <thread>

struct MatchResult {
    int posX;
    int posY;
    int width;
    int height;
    double accuracy;

    //First color
    int fColorR;
    int fColorG;
    int fColorB;

    //Second color
    int sColorR;
    int sColorG;
    int sColorB;
};

class core {
public:
    core();
    ~core();

    bool generate(wxString filePath);

    bool clearImages(std::string folderPath);

    bool matchEveryElement();

    bool drawAllRectangles();

private:
    bool findMatchingElements(wxString croppedImage, wxString mainImage, wxColour fColor, wxColour sColor);

    bool saveToJson(std::vector<MatchResult> results);

    bool firstDrawing = true;

    int jsonCounter = 1;

    bool drawRectanglesFromJson(wxString jsonPath);

    bool firstSaving = true;
};



#endif //CORE_H

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
#include <mutex>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/ximgproc/slic.hpp>

struct MatchResult {
    int decomposedID;
    int croppedID;
    int posX;
    int posY;
    int width;
    int height;
    double accuracy;
    double scale;
    double rotation;

    int fColor;
    int sColor;
};

class core {
public:
    core();
    ~core();

    bool generate(wxString filePath);

    bool clearImages(std::string folderPath);

    bool matchEveryElement();

    bool drawAllRectangles(double accuracy);

    std::vector<wxColour> colors = {
        wxColour(255, 0, 0),
        wxColour(0, 255, 0),
        wxColour(0, 0, 255),
        wxColour(255, 255, 0),
        wxColour(0, 255, 255),
        wxColour(255, 0, 255),
        wxColour(255, 165, 0),
        wxColour(128, 0, 128)
    };

    bool drawRectanglesFromJson(wxString jsonPath, double accuracy, bool accuracyDrawing);

    bool saveWithNewAccuracy(wxString existingJsonPath, wxString newJsonPath, double accuracy);

    int croppedImageID = 1;
    int decomposedImageID = 1;

private:
    bool findMatchingElements(wxString croppedImage, wxString mainImage, int fColor, int sColor);

    bool saveToJson(std::vector<MatchResult> results, bool withCounter);

    bool NMSForAllResultsCombined();

    bool firstDrawing = true;

    int jsonCounter = 1;

    bool firstSaving = true;

    int tasksCounter = 0;
    int completedTasksCounter = 0;
};



#endif //CORE_H

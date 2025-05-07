#ifndef CORE_H
#define CORE_H

#include "imageProcessing.h"
#include <vector>

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

struct MatchResult {
    int posX;
    int posY;
    int width;
    int height;
    double accuracy;
};

class core {
public:
    core();
    ~core();

    bool generate(wxString filePath);

    bool clearImages(std::string folderPath);

    std::vector<std::vector<MatchResult>> matchEveryElement();

private:
    std::vector<MatchResult> findMatchingElements(wxString croppedImage, wxString mainImage);

    bool straightUpImage(std::string filePath);
    bool generateImages();
};



#endif //CORE_H

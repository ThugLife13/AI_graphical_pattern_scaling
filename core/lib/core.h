#ifndef CORE_H
#define CORE_H

#include "imageProcessing.h"

namespace fs = std::filesystem;

class core {
public:
    core();
    ~core();

    bool generate(wxString filePath);
private:
    bool straightUpImage(std::string filePath);
    bool generateImages();
    bool clearImages();
};



#endif //CORE_H

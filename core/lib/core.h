#ifndef CORE_H
#define CORE_H

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/filepicker.h>
#include <wx/image.h>
#include <wx/filectrl.h>
#include <wx/stdpaths.h>
#include <wx/string.h>

#include <iostream>
#include <string>
#include <map>
#include <filesystem>

#include <spdlog/spdlog.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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

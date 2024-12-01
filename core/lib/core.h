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

#include <spdlog/spdlog.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class core {
public:
    core();
    ~core();

    bool generate(wxString filePath);
private:
    bool straightUpImage(std::string filePath);
    bool generateImages();
};



#endif //CORE_H

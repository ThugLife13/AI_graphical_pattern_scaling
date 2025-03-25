#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

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
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

class imageProcessing {
public:
    bool procesImages(wxString filePath);
    bool createGrayScale(wxString filePath);
    bool createColorPalette(wxString filePath);
};



#endif //IMAGEPROCESSING_H

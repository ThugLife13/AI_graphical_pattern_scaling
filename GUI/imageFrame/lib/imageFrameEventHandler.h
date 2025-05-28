#ifndef IMAGEFRAMEEVENTHANDLER_H
#define IMAGEFRAMEEVENTHANDLER_H

#include "../../decompositionFrame/lib/decompositionFrame.h"
#include <filesystem>

class core;

class imageFrame;

class imageFrameEventHandler {
public:
    imageFrameEventHandler(imageFrame* frame, bool& newWindowOpened) : frame(frame), newWindowOpened(newWindowOpened) {}

    bool fileOpened = false;

    void registerImageComponents(wxPanel* panel, wxStaticBitmap* display, const wxImage& image);

    void onResize(wxSizeEvent& evt);
    void onCloseWindow(wxCloseEvent& evt);
    void onGenerateButtonClicked(wxCommandEvent& evt);
    void onDecomposeButtonClicked(wxCommandEvent& evt);
    void onRadioBoxChanged(wxCommandEvent& evt);

private:
    imageFrame* frame;
    wxPanel* imagePanel;
    wxPanel* leftPanel;
    wxStaticBitmap* imageDisplay;
    wxImage originalImage;
    wxImage defaultImage;
    
    wxImage currentImage;

    core* newCore;

    void sizeCorrection(wxSize panelSize);
    void updateRadioBox();

    bool decompositionWindowOpened = false;

    bool& newWindowOpened;
};


#endif //IMAGEFRAMEEVENTHANDLER_H

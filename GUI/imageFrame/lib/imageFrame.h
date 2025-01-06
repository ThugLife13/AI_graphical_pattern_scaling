#ifndef IMAGE_WINDOW_H
#define IMAGE_WINDOW_H

#include "imageFrameEventHandler.h"

class imageFrame : public wxFrame {
public:
    imageFrame(wxWindow* parent, const wxString& filePath, bool& newWindowOpened);

    wxString originalFilePath;

    wxImage originalImage;

    wxRadioBox* radioBox;

    wxArrayString radioChoices;

    wxPanel* panelLeft;

    wxPanel* panelRight;

private:
    wxStaticBitmap* imageDisplay;
    imageFrameEventHandler ifehandlers;
};

#endif // IMAGE_WINDOW_H
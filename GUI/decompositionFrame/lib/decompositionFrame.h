//
// Created by mateu on 28.03.2025.
//

#ifndef DECOMPOSITIONFRAME_H
#define DECOMPOSITIONFRAME_H

#include "decompositionFrameEventHandler.h"
#include "../../../core/lib/imagePanel.h"

class decompositionFrame : public wxFrame {
public:
    decompositionFrame(wxWindow* parent, const wxString& filePath, bool& decompositionWindowOpened);
    ~decompositionFrame();

    void applyZoom(double zoom);
    wxBitmap getCurrentBitmap();

    wxScrolledWindow* leftPanel;
    imagePanel* imageDisplay;

    bool decompositionWindowOpened;
    decompositionFrameEventHandler dfehandlers;
    wxImage originalImage;
    wxBitmap originalBitmap;
    double currentZoom;

    wxSlider* scaleSlider;

    wxButton* endDecompositionPhaseButton;
    wxButton* analyseImagesButton;
    wxButton* clearButton;
    wxButton* copyButton;

    wxChoice* choiceBox;

    wxBoxSizer* rightSizer;

    wxPanel* rightPanel;

    std::vector<wxString> decomposedImages;
};



#endif //DECOMPOSITIONFRAME_H

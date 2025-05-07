#ifndef DECOMPOSITIONFRAMEEVENTHANDLER_H
#define DECOMPOSITIONFRAMEEVENTHANDLER_H

#include "../../../core/lib/core.h"
#include <wx/event.h>
#include <iostream>
#include <filesystem>

class decompositionFrame;

class decompositionFrameEventHandler : public wxEvtHandler {
public:
    decompositionFrameEventHandler(decompositionFrame* frame, bool& decompositionWindowOpened);
    void scaleSliderEventHandler(wxCommandEvent& event);
    void onClearButton(wxCommandEvent& event);
    void onCopyButton(wxCommandEvent& event);
    void onEndDecompositionPhaseButton(wxCommandEvent& event);
    void onChooseButton(wxCommandEvent& event);
    void onAnalyseButton(wxCommandEvent& event);
private:
    decompositionFrame* frame;
    bool& decompositionWindowOpened;
    int lastSliderValue;
    int cropped = 1;
    int decomposed = 1;
    int workingImage = 1;

    bool decomposing = true;
};

#endif // DECOMPOSITIONFRAMEEVENTHANDLER_H

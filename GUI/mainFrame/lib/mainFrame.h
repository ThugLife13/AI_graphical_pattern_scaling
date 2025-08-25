#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "mainFrameEventHandlers.h"
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/filepicker.h>

enum PHASE {
    PREP = 0,
    AI_RECOGNITION,
    AI_GENERATION,
};

class mainFrame : public wxFrame {
public:
    mainFrame(const wxString &title);
    ~mainFrame();

    void switchPhase(PHASE newPhase);

    void getImageSize();

    int imageSizeX;
    int imageSizeY;

    enum PHASE currentPhase = PREP;

    wxPanel* mainPanel;
    wxBoxSizer* mainSizer;

    wxBoxSizer* leftSizer;
    wxBoxSizer* rightSizer;

    wxPanel* leftPanel;
    wxPanel* rightPanel;

    wxChoice* choiceBox;

    //PREP PHASE
    wxString path = " ";

    wxStaticText* errorText1;
    wxStaticText* errorText2;
    wxFilePickerCtrl* filePicker;

    //AI_RECOGNITION PHASE
    wxButton* startRecognition;

    //AI_GENERATION PHASE
    wxStaticText* newSizeX;
    wxStaticText* nexSizeY;
    wxSpinCtrl* sizeXSpinCtrl;
    wxSpinCtrl* sizeYSpinCtrl;
    wxButton* startGeneration;

private:
    mainFrameEventHandlers mfehandlers;
};



#endif //MAINFRAME_H

#ifndef MAINFRAMEEVENTHANDLERS_H
#define MAINFRAMEEVENTHANDLERS_H

#include <../../../../GUI/imageFrame/lib/imageFrame.h>

class mainFrame;

class mainFrameEventHandlers {
public:
    mainFrameEventHandlers(mainFrame* frame) : frame(frame) {}

    void onChoice(wxCommandEvent& event);

    //PREP PHASE
    void openFileButtonHandler(wxCommandEvent& evt);
    void openFilePickerHandler(wxCommandEvent& evt);
    bool fileOpened = false;
    bool newWindowOpened = false;

    //AI_RECOGNITION PHASE
    void onSizeXSpinCtrl(wxCommandEvent& event);
    void onSizeYSpinCtrl(wxCommandEvent& event);
    void startRecognition(wxCommandEvent& evt);

    //AI_GENERATION PHASE
private:
    int selectedSizeX = 0;
    int selectedSizeY = 0;
    mainFrame* frame;
};



#endif //MAINFRAMEEVENTHANDLERS_H

#ifndef MAINFRAMEEVENTHANDLERS_H
#define MAINFRAMEEVENTHANDLERS_H

#include <../../../../GUI/imageFrame/lib/imageFrame.h>

class mainFrame;

class mainFrameEventHandlers {
public:
    mainFrameEventHandlers(mainFrame* frame) : frame(frame) {}

    void openFileButtonHandler(wxCommandEvent& evt);
    void openFilePickerHandler(wxCommandEvent& evt);
    bool fileOpened = false;
    bool newWindowOpened = false;
private:
    mainFrame* frame;
};



#endif //MAINFRAMEEVENTHANDLERS_H

#include "../lib/mainFrameEventHandlers.h"
#include "../lib/mainFrame.h"


void mainFrameEventHandlers::openFileButtonHandler(wxCommandEvent& evt) {
    //opening new window with image displayed
    spdlog::info("openFileButtonHandler: Opening new window with chosen file button");


    wxInitializer initializer;
    if (!initializer) {
        spdlog::error("openFileButtonHandler: Failed to initialize wxWidgets!");
    }
    wxInitAllImageHandlers();

    if (fileOpened == false) {
        frame->errorText1->Show();
        spdlog::error("openFileButtonHandler: File not opened!");
    } else {
        if (newWindowOpened == true) {
            frame->errorText2->Show();
            spdlog::error("openFileButtonHandler: New window already opened!");
        } else {
            newWindowOpened = true;
            spdlog::info("openFileButtonHandler: Creating new image window");
            imageFrame* newWindow = new imageFrame(frame, frame->path, newWindowOpened);
            newWindow->Show();
            frame->errorText2->Hide();
        }


    }
}


void mainFrameEventHandlers::openFilePickerHandler(wxCommandEvent& evt) {
    if (frame) {
        frame->path = frame->filePicker->GetPath();
        spdlog::info("openFilePickerHandler: File selected: {}", frame->path.ToStdString());
        fileOpened = true;
        frame->errorText1->Hide();
    }
}
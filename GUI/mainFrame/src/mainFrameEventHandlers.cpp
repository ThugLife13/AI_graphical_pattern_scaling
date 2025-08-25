#include "../lib/mainFrameEventHandlers.h"
#include "../lib/mainFrame.h"

void mainFrameEventHandlers::onChoice(wxCommandEvent &event) {
    std::string choice = event.GetString().ToStdString();

    if (choice == "PREP") {
        frame->switchPhase(PREP);
    } else if (choice == "AI_RECOGNITION") {
        frame->switchPhase(AI_RECOGNITION);
    } else if (choice == "AI_GENERATION") {
        frame->switchPhase(AI_GENERATION);
    }
}

//PREP PHASE
void mainFrameEventHandlers::openFileButtonHandler(wxCommandEvent& evt) {
    //opening new window with image displayed
    spdlog::info("openFileButtonHandler: Opening new window with chosen file");

    wxInitializer initializer;
    if (!initializer) {
        spdlog::error("openFileButtonHandler: Failed to initialize wxWidgets!");
    }
    wxInitAllImageHandlers();

    if (fileOpened == false) {
        frame->errorText1->Show();
        frame->leftSizer->Layout();
        spdlog::error("openFileButtonHandler: File not opened!");
    } else {
        if (newWindowOpened == true) {
            frame->errorText2->Show();
            frame->leftSizer->Layout();
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

void mainFrameEventHandlers::onSizeXSpinCtrl(wxCommandEvent &evt) {
    selectedSizeX = evt.GetInt();
}

void mainFrameEventHandlers::onSizeYSpinCtrl(wxCommandEvent &evt) {
    selectedSizeY = evt.GetInt();
}

void mainFrameEventHandlers::startGeneration(wxCommandEvent &evt) {
    spdlog::info("startGeneration: Starting generation");
    spdlog::info("startGeneration: Size X: {}", selectedSizeX);
    spdlog::info("startGeneration: Size Y: {}", selectedSizeY);

    aiGeneration* aiGen = new aiGeneration();
    aiGen->startGeneration(selectedSizeX, selectedSizeY);
}

//AI_RECOGNITION PHASE
void mainFrameEventHandlers::startRecognition(wxCommandEvent &evt){
    aiRecognition* aiRec = new aiRecognition();
    aiRec->startRecognition();
}
//AI_GENERATION PHASE
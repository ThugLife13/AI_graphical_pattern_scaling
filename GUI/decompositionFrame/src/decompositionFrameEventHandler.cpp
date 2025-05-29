#include "../lib/decompositionFrameEventHandler.h"
#include "../lib/decompositionFrame.h"

namespace fs = std::filesystem;

decompositionFrameEventHandler::decompositionFrameEventHandler(decompositionFrame* frame, bool& decompositionWindowOpened)
    : frame(frame), decompositionWindowOpened(decompositionWindowOpened), lastSliderValue(100) {}

void decompositionFrameEventHandler::scaleSliderEventHandler(wxCommandEvent &event) {
    int currentValue = event.GetInt();
    if (currentValue == lastSliderValue) return;

    lastSliderValue = currentValue;
    double zoom = currentValue / 100.0;
    frame->applyZoom(zoom);
}

void decompositionFrameEventHandler::onClearButton(wxCommandEvent& event) {
    frame->imageDisplay->clearSelection();
}

void decompositionFrameEventHandler::onCopyButton(wxCommandEvent& event) {


    wxRect selection = frame->imageDisplay->getSelection();

    if (selection.IsEmpty()) return;

    selection = selection.Intersect(wxRect(0, 0, frame->originalImage.GetWidth(), frame->originalImage.GetHeight()));

    wxImage croppedImage = frame->originalImage.GetSubImage(selection);


if (cropped == 1 && decomposed == 1) {
    core Core;
    Core.clearImages("..\\tmp\\cropped");
    Core.clearImages("..\\tmp\\decomposedImages");
    fs::copy_file(fs::path("..\\tmp\\images\\grayQuantized.jpg"),
        fs::path("..\\tmp\\images\\toDecompose.jpg"),
        fs::copy_options::overwrite_existing);
}

    if (!decomposing) {
        std::string path = "../tmp/cropped/" + frame->decomposedImages[workingImage].ToStdString() + "/cropped_" + std::to_string(cropped++) + ".jpg";
        croppedImage.SaveFile(path, wxBITMAP_TYPE_JPEG);
        spdlog::info("onCopyButton: Image cropped");
    }
    else if (decomposing) {
        if (decomposed == 1) {
            frame->endDecompositionPhaseButton->Show(true);
            frame->rightPanel->Layout();
            frame->rightPanel->Refresh();
        }

        std::string path = "../tmp/decomposedImages/decomposed_" + std::to_string(decomposed) + ".jpg";
        croppedImage.SaveFile(path, wxBITMAP_TYPE_JPEG);

        wxRect originalSelection = frame->imageDisplay->getSelection();
        bool success = imageProcessing::fillSelectionWithWhite(originalSelection);

        if (success) {
            frame->originalImage.LoadFile("../tmp/images/toDecompose.jpg");
            frame->originalBitmap = wxBitmap(frame->originalImage);
            frame->imageDisplay->setBitmap(frame->originalBitmap, frame->currentZoom);
            frame->imageDisplay->Refresh();
        }

        decomposed++;
        spdlog::info("onCopyButton: Image decomposedImages");
    }
}

void decompositionFrameEventHandler::onEndDecompositionPhaseButton(wxCommandEvent &event) {
    frame->endDecompositionPhaseButton->Show(false);
    frame->decomposedImages.clear();

    decomposing = false;
    spdlog::info("onRadioBox: Selected cropping");

    for (const auto& entry : fs::directory_iterator("../tmp/decomposedImages")) {
        if (entry.is_regular_file()) {
            fs::path folder_name = entry.path().stem();
            fs::create_directory("../tmp/cropped" / folder_name);
            frame->decomposedImages.push_back(folder_name.string());
        }
    }

    frame->choiceBox->Clear();
    for (const wxString& item : frame->decomposedImages) {
        frame->choiceBox->Append(item);
    }

    frame->analyseImagesButton->Show(true);
    frame->choiceBox->Show(true);
    frame->rightPanel->Layout();
    frame->rightPanel->Refresh();
}

void decompositionFrameEventHandler::onChooseButton(wxCommandEvent &event) {
    workingImage = event.GetInt();
    spdlog::info(frame->decomposedImages[event.GetInt()].ToStdString());

    std::string tmpPath = "../tmp/decomposedImages/" + frame->decomposedImages[workingImage].ToStdString() + ".jpg";

    frame->originalImage.LoadFile(tmpPath);
    frame->originalBitmap = wxBitmap(frame->originalImage);
    frame->imageDisplay->setBitmap(frame->originalBitmap, frame->currentZoom);
    frame->imageDisplay->Refresh();
}

void decompositionFrameEventHandler::onAnalyseButton(wxCommandEvent &event) {
    fs::copy_file(fs::path("..\\tmp\\images\\grayQuantized.jpg"),
        fs::path("..\\tmp\\images\\forAnalyzing.jpg"),
        fs::copy_options::overwrite_existing);

    core Core;

    //TODO - unhash for analyzing
    Core.matchEveryElement();

    //TODO - unhash for drawing
    Core.drawAllRectangles();

    frame->choiceBox->Show(false);
    frame->endDecompositionPhaseButton->Show(false);
    frame->copyButton->Show(false);
    frame->clearButton->Show(false);
    frame->rightPanel->Layout();
    frame->rightPanel->Refresh();

    frame->originalImage.LoadFile("../tmp/images/analyzed.jpg");
    frame->originalBitmap = wxBitmap(frame->originalImage);
    frame->imageDisplay->clearSelection();
    frame->imageDisplay->setBitmap(frame->originalBitmap, frame->currentZoom);
    frame->imageDisplay->Refresh();

}

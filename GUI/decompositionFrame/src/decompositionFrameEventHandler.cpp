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
    currentZoom = zoom;
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

    //2'nd view
    frame->analyseImagesButton->Show(true);
    frame->choiceBox->Show(true);
    frame->rightPanel->Layout();
    frame->rightPanel->Refresh();
}

void decompositionFrameEventHandler::onChooseButton(wxCommandEvent &event) {
    if (beforeAnalysing) {
        workingImage = event.GetInt();
        spdlog::info(frame->decomposedImages[event.GetInt()].ToStdString());

        std::string tmpPath = "../tmp/decomposedImages/" + frame->decomposedImages[workingImage].ToStdString() + ".jpg";

        frame->originalImage.LoadFile(tmpPath);
        frame->originalBitmap = wxBitmap(frame->originalImage);
        frame->imageDisplay->setBitmap(frame->originalBitmap, frame->currentZoom);
        frame->imageDisplay->Refresh();
    }
    else {
        workingImage = event.GetInt();
        spdlog::info(frame->drawnImages[event.GetInt()].ToStdString());

        std::string tmpPath = "../tmp/analyzed/" + frame->drawnImages[workingImage].ToStdString() + ".jpg";

        frame->originalImage.LoadFile(tmpPath);
        frame->originalBitmap = wxBitmap(frame->originalImage);
        frame->imageDisplay->setBitmap(frame->originalBitmap, frame->currentZoom);
        frame->imageDisplay->Refresh();

        if ("../tmp/analyzed/" + frame->drawnImages[workingImage].ToStdString() + ".jpg" == "../tmp/analyzed/analyzedCombined.jpg") {
            frame->accuracySpinCtrl->Show(false);
            frame->saveNewAccuracyIntoJsonButton->Show(false);
            frame->rightPanel->Layout();
            frame->rightPanel->Refresh();
        }
        else {
            frame->accuracySpinCtrl->Show(true);
            frame->saveNewAccuracyIntoJsonButton->Show(true);
            frame->rightPanel->Layout();
            frame->rightPanel->Refresh();
        }

        //on switch returning to base accuracy
        frame->accuracySpinCtrl->SetValue(selectedBaseAccuracy);
    }
}

void decompositionFrameEventHandler::onAnalyseButton(wxCommandEvent &event) {
    fs::copy_file(fs::path("..\\tmp\\images\\grayQuantized.jpg"),
        fs::path("..\\tmp\\images\\forAnalyzing.jpg"),
        fs::copy_options::overwrite_existing);

    frame->choiceBox->Show(false);
    frame->endDecompositionPhaseButton->Show(false);
    frame->copyButton->Show(false);
    frame->clearButton->Show(false);
    frame->analyseImagesButton->Show(false);

    core Core;

    //TODO - unhash for analyzing
    Core.matchEveryElement();

    //TODO - unhash for drawing
    Core.drawAllRectangles(selectedBaseAccuracy);

    beforeAnalysing = false;

    updateChoiceBoxDrawn();
    frame->accuracySpinCtrl->Show(true);

    frame->originalImage.LoadFile("../tmp/analyzed/analyzedCombined.jpg");
    frame->originalBitmap = wxBitmap(frame->originalImage);

    frame->imageDisplay->clearSelection();
    frame->imageDisplay->setBitmap(frame->originalBitmap, frame->currentZoom);

    frame->rightPanel->Layout();
    frame->rightPanel->Refresh();
    frame->imageDisplay->Refresh();
}

void decompositionFrameEventHandler::updateChoiceBoxDrawn() {
    frame->drawnImages.clear();
    core Core;

    wxString tmpString = "analyzedCombined";

    spdlog::info("updateChoiceBoxDrawn: Selected cropping");

    frame->drawnImages.push_back(tmpString);

    for (const auto& entry : fs::directory_iterator("..\\tmp\\jsons")) {
        if (entry.is_regular_file()) {
            spdlog::info("updateChoiceBoxDrawn: Full path {}", entry.path().string());
            fs::path folder_name = entry.path().stem();

            //fs::create_directory("../tmp/cropped" / folder_name);
            Core.drawRectanglesFromJson(folder_name.string(), 0.5, true);

            spdlog::info("updateChoiceBoxDrawn: Short name: {}", folder_name.string());
            frame->drawnImages.push_back(folder_name.string());
        }
    }

    frame->choiceBoxDrawn->Clear();
    for (const wxString& item : frame->drawnImages) {
        frame->choiceBoxDrawn->Append(item);
    }

    frame->choiceBoxDrawn->Show(true);
    frame->rightPanel->Layout();
    frame->rightPanel->Refresh();
}

void decompositionFrameEventHandler::onAccuracySpinCtrl(wxCommandEvent &event) {
    tmpAccuracy = event.GetInt() / 100.0f;

    core Core;
    Core.drawRectanglesFromJson(frame->drawnImages[workingImage].ToStdString(), tmpAccuracy, true);

    std::string tmpPath = "../tmp/analyzed/" + frame->drawnImages[workingImage].ToStdString() + ".jpg";
    frame->originalImage.LoadFile(tmpPath);
    frame->originalBitmap = wxBitmap(frame->originalImage);
    frame->imageDisplay->setBitmap(frame->originalBitmap, frame->currentZoom);
    frame->imageDisplay->Refresh();
    frame->applyZoom(currentZoom);
}

void decompositionFrameEventHandler::onSaveAccuracyButton(wxCommandEvent &event) {

    fs::create_directory("../tmp/jsons/newAccuracy");

    core Core;
    Core.saveWithNewAccuracy("../tmp/jsons/" + frame->drawnImages[workingImage] + ".json",
        "../tmp/jsons/newAccuracy/" + frame->drawnImages[workingImage] + ".json", tmpAccuracy);

    frame->endSavingAccuracyPhaseButton->Show(true);
    frame->rightPanel->Layout();
    frame->rightPanel->Refresh();
}

void decompositionFrameEventHandler::onEndSavingAccuracyPhaseButton(wxCommandEvent &event) {
    std::vector<wxString> tmp;
    std::vector<wxString> missing;
    for (const auto& entry : fs::directory_iterator("..\\tmp\\jsons\\newAccuracy")) {
        if (entry.is_regular_file()) {
            tmp.push_back(entry.path().stem().string());
        }
    }

    for (const auto& image : frame->drawnImages) {
        bool appeared = false;
        if (image == "analyzedCombined") {
            continue;
        }

        for (const auto& path : tmp) {
            if (path.ToStdString() == image) {
                appeared = true;
            }
        }
        if (!appeared) {
            missing.push_back(image);
        }
    }

    for (const auto& toCopy : missing) {
        std::string from;
        std::string to;

        from = "..\\tmp\\jsons\\" + toCopy + ".json";
        to = "..\\tmp\\jsons\\newAccuracy\\" + toCopy + ".json";

        spdlog::info("onEndSavingAccuracyPhaseButton: Copying from: {}, to: {}", from, to);
        fs::copy_file(from,to);
    }

    //selecting image to display
    std::string tmpPath = "../tmp/analyzed/analyzedCombined.jpg";
    frame->originalImage.LoadFile(tmpPath);
    frame->originalBitmap = wxBitmap(frame->originalImage);
    frame->imageDisplay->setBitmap(frame->originalBitmap, frame->currentZoom);
    frame->imageDisplay->Refresh();

    wxTheApp->Exit();
}

#include "../lib/imageFrameEventHandler.h"
#include "../lib/imageFrame.h"
#include "../../../core/lib/core.h"

void imageFrameEventHandler::onCloseWindow(wxCloseEvent &evt) {
    newWindowOpened = false;
    evt.Skip();
}

void imageFrameEventHandler::registerImageComponents(wxPanel* panel, wxStaticBitmap* display, const wxImage& image) {
    imagePanel = panel;
    imageDisplay = display;
    originalImage = image;
    defaultImage = image;

    currentImage = originalImage;

    imagePanel->Bind(wxEVT_SIZE, &imageFrameEventHandler::onResize, this);
}

void imageFrameEventHandler::onResize(wxSizeEvent& event) {
    if (!imagePanel || !imageDisplay || !originalImage.IsOk()) {
        event.Skip();
        return;
    }
    
    wxSize panelSize = imagePanel->GetSize();

    if (panelSize.x <= 0 || panelSize.y <= 0) {
        event.Skip();
        return;
    }

    sizeCorrection(panelSize);
    event.Skip();
}


void imageFrameEventHandler::onButtonClicked(wxCommandEvent& evt) {
    spdlog::info("onButtonClicked: Button Clicked");

    if (newCore->generate(frame->originalFilePath)) {
        spdlog::info("onButtonClicked: Images generated. Updating radioBox");
        updateRadioBox();
        spdlog::info("onButtonClicked: RadioBox updated");

    }
}

void imageFrameEventHandler::onRadioBoxChanged(wxCommandEvent& evt) {
    if (!imageDisplay || !imagePanel) return;

    int selection = evt.GetSelection();
    if (selection < 0 || selection >= static_cast<int>(frame->radioChoices.size())) {
        spdlog::info("onRadioBoxChanged: Invalid selection index: {}", selection);
        return;
    }
    wxString filePath;

    std::string selectedFile = frame->radioChoices[selection].ToStdString();
    if (frame->radioChoices[selection].ToStdString() == "Original image") {
        filePath = frame->originalFilePath;
    } else {
        spdlog::info("onRadioBoxChanged: Selected file: {}", selectedFile);

        wxString folderPath = wxStandardPaths::Get().GetExecutablePath();
        folderPath = wxPathOnly(folderPath) + "\\..\\tmp\\images\\";
        filePath = folderPath + selectedFile;
    }

    wxImage newImage;
    if (!newImage.LoadFile(filePath)) {
        spdlog::error("onRadioBoxChanged: Failed to load image from: {}", filePath.ToStdString());
        return;
    }

    if (newImage.IsOk()) {
        currentImage = newImage;
        wxBitmap bitmap(newImage.Scale(imagePanel->GetSize().x, imagePanel->GetSize().y, wxIMAGE_QUALITY_HIGH));
        imageDisplay->SetBitmap(bitmap);
        sizeCorrection(imagePanel->GetSize());
        spdlog::info("onRadioBoxChanged: Image updated successfully.");
    } else {
        spdlog::error("onRadioBoxChanged: Loaded image is not valid.");
    }
}

void imageFrameEventHandler::sizeCorrection(wxSize panelSize) {
    wxSize imgSize = currentImage.GetSize();

    double imgAspect = static_cast<double>(imgSize.x) / imgSize.y;
    double panelAspect = static_cast<double>(panelSize.x) / panelSize.y;
    wxSize newSize;

    if (panelAspect > imgAspect) {
        newSize.SetHeight(panelSize.y);
        newSize.SetWidth(static_cast<int>(panelSize.y * imgAspect));
    } else {
        newSize.SetWidth(panelSize.x);
        newSize.SetHeight(static_cast<int>(panelSize.x / imgAspect));
    }

    wxImage scaledImage = currentImage.Scale(newSize.GetWidth(), newSize.GetHeight(), wxIMAGE_QUALITY_HIGH);
    imageDisplay->SetBitmap(wxBitmap(scaledImage));

    wxPoint position((panelSize.x - newSize.x) / 2, (panelSize.y - newSize.y) / 2);
    imageDisplay->SetPosition(position);
}

void imageFrameEventHandler::updateRadioBox() {
    spdlog::info("updateRadioBox: Updating radio box.");

    if (frame->radioBox) {
        frame->radioBox->Destroy();
    }

    std::string folderPath = "..\\tmp\\images";
    frame->radioChoices.clear();
    frame->radioChoices.push_back("Original image");
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (fs::is_regular_file(entry.path())) {
            frame->radioChoices.push_back(entry.path().filename().string());
        }
    }

    wxArrayString choices;
    for (const auto& choice : frame->radioChoices) {
        choices.Add(choice);
    }

    frame->radioBox = new wxRadioBox(frame->panelLeft, wxID_ANY, "Choose an Image", wxPoint(10, 150), wxDefaultSize, choices, 1, wxRA_SPECIFY_COLS);
    frame->radioBox->Bind(wxEVT_RADIOBOX, &imageFrameEventHandler::onRadioBoxChanged, this);

    frame->panelLeft->Layout();
    frame->radioBox->Show();
}
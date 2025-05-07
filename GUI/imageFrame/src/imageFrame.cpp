#include "../lib/imageFrame.h"

imageFrame::imageFrame(wxWindow* parent, const wxString& filePath, bool &newWindowOpened)
    : wxFrame(parent, wxID_ANY, "Image Viewer", wxDefaultPosition, wxSize(900, 600)), ifehandlers(this, newWindowOpened) {
    originalFilePath = filePath;

    wxPanel* panel = new wxPanel(this, wxID_ANY);

    panelLeft = new wxPanel(panel, wxID_ANY, wxPoint(1, 1), wxSize(300, panel->GetSize().y));
    panelRight = new wxPanel(panel, wxID_ANY, wxPoint(301, 1), wxSize((panel->GetSize().x)-300, panel->GetSize().y));

    wxStaticBitmap* imageDisplay = nullptr;


    // Load image
    if (originalImage.LoadFile(filePath)) {
        if (!originalImage.IsOk()) {
            spdlog::error("imageFrame: Image is not valid.");
            return;
        }

        wxBitmap bitmap(originalImage);
        imageDisplay = new wxStaticBitmap(panelRight, wxID_ANY, bitmap);

        // Register the imageDisplay and the panelRight in the event handler
        ifehandlers.registerImageComponents(panelRight, imageDisplay, originalImage);

    } else {
        spdlog::error("imageFrame: Error loading image");
        spdlog::info("imageFrame: Bad Path: {}", filePath.ToStdString());
    }
    
    //Radio
    radioBox = new wxRadioBox(panelLeft, wxID_ANY, "Choose an Image", wxPoint(10, 150), wxDefaultSize, radioChoices, 1, wxRA_SPECIFY_COLS);
    radioBox->Bind(wxEVT_RADIOBOX, &imageFrameEventHandler::onRadioBoxChanged, &ifehandlers);
    radioBox->Hide();

    wxButton* generateButton = new wxButton(panelLeft, wxID_ANY, "Generate images", wxPoint(10, 10), wxSize(150, 30));
    generateButton->Bind(wxEVT_BUTTON, &imageFrameEventHandler::onGenerateButtonClicked, &ifehandlers);

    decomposeButton = new wxButton(panelLeft, wxID_ANY, "Decompose image", wxPoint(10, 50), wxSize(150, 30));
    decomposeButton->Bind(wxEVT_BUTTON, &imageFrameEventHandler::onDecomposeButtonClicked, &ifehandlers);
    decomposeButton->Show(false);

    // Layout
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(panelLeft, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(panelRight, 1, wxEXPAND | wxALL, 5);

    panel->SetSizerAndFit(mainSizer);

    this->Bind(wxEVT_CLOSE_WINDOW, &imageFrameEventHandler::onCloseWindow, &ifehandlers);
}
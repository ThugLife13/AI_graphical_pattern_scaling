#include "../lib/decompositionFrame.h"

decompositionFrame::decompositionFrame(wxWindow *parent, const wxString &filePath, bool &decompositionWindowOpened)
: wxFrame(parent, wxID_ANY, "Decompose", wxDefaultPosition, wxSize(900, 600)),
  dfehandlers(this, decompositionWindowOpened),
  currentZoom(1.0) {
    decomposedImages.push_back("null");

    wxPanel* mainPanel = new wxPanel(this, wxID_ANY);
    mainPanel->SetMinSize(this->GetSize());
    mainPanel->SetBackgroundColour(wxColour(180, 255, 220));

    wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);
    mainPanel->SetSizer(mainSizer);

    leftPanel = new wxScrolledWindow(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE | wxHSCROLL | wxVSCROLL);
    leftPanel->SetScrollRate(10, 10);
    mainSizer->Add(leftPanel, 4, wxEXPAND);

    rightPanel = new wxPanel(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    mainSizer->Add(rightPanel, 1, wxEXPAND);


    rightSizer = new wxBoxSizer(wxVERTICAL);
    rightPanel->SetSizer(rightSizer);

    choiceBox = new wxChoice(rightPanel, wxID_ANY);
    rightSizer->Add(choiceBox, 0, wxALL, 10);
    choiceBox->Show(false);

    scaleSlider = new wxSlider(rightPanel, wxID_ANY, 100, 10, 200, wxPoint(10, 10), wxSize(150, 30), wxHORIZONTAL);
    rightSizer->Add(scaleSlider, 0, wxALL | wxEXPAND, 10);

    clearButton = new wxButton(rightPanel, wxID_ANY, "Clear Selection");
    rightSizer->Add(clearButton, 0, wxALL, 10);

    copyButton = new wxButton(rightPanel, wxID_ANY, "Copy Selection");
    rightSizer->Add(copyButton, 0, wxALL, 10);

    analyseImagesButton = new wxButton(rightPanel, wxID_ANY, "Analyse images");
    rightSizer->Add(analyseImagesButton, 0, wxALL, 10);
    analyseImagesButton->Show(false);

    endDecompositionPhaseButton = new wxButton(rightPanel, wxID_ANY, "End Decomposition phase");
    rightSizer->Add(endDecompositionPhaseButton, 0, wxALL | wxEXPAND, 10);
    endDecompositionPhaseButton->Show(false);


    if (originalImage.LoadFile(filePath)) {
        originalBitmap = wxBitmap(originalImage);

        leftPanel->SetSizer(new wxBoxSizer(wxVERTICAL));

        imageDisplay = new imagePanel(leftPanel, this);
        imageDisplay->setBitmap(originalBitmap, currentZoom);

        leftPanel->GetSizer()->Add(imageDisplay, 1, wxEXPAND | wxALL, 0);

        leftPanel->FitInside();
        leftPanel->Layout();
        leftPanel->Refresh();
    }

    clearButton->Bind(wxEVT_BUTTON, &decompositionFrameEventHandler::onClearButton, &dfehandlers);
    copyButton->Bind(wxEVT_BUTTON, &decompositionFrameEventHandler::onCopyButton, &dfehandlers);
    scaleSlider->Bind(wxEVT_SLIDER, &decompositionFrameEventHandler::scaleSliderEventHandler, &dfehandlers);
    endDecompositionPhaseButton->Bind(wxEVT_BUTTON, &decompositionFrameEventHandler::onEndDecompositionPhaseButton, &dfehandlers);
    choiceBox->Bind(wxEVT_CHOICE, &decompositionFrameEventHandler::onChooseButton, &dfehandlers);
    analyseImagesButton->Bind(wxEVT_BUTTON, &decompositionFrameEventHandler::onAnalyseButton, &dfehandlers);
}

void decompositionFrame::applyZoom(double zoom) {
    currentZoom = zoom;

    wxImage tempImage = originalImage.Copy();
    tempImage.Rescale(originalImage.GetWidth() * zoom, originalImage.GetHeight() * zoom);
    wxBitmap scaledBitmap(tempImage);

    imageDisplay->setBitmap(scaledBitmap, currentZoom);

    imageDisplay->SetMinSize(wxSize(scaledBitmap.GetWidth(), scaledBitmap.GetHeight()));

    leftPanel->FitInside();
    leftPanel->Scroll(0, 0);
    leftPanel->Refresh();

}

wxBitmap decompositionFrame::getCurrentBitmap() {
    return imageDisplay->getBitmap();
}

decompositionFrame::~decompositionFrame() {}

/*
 * Copy Selection zamienić na Decompose
 * Dodać przycisk "z
 */
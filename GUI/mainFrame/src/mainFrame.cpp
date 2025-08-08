#include "../lib/mainFrame.h"

namespace fs = std::filesystem;

mainFrame::mainFrame(const wxString &title): wxFrame(nullptr, wxID_ANY, title, wxPoint(10,10), wxSize(800, 300)), mfehandlers(this) {

    mainPanel = new wxPanel(this, wxID_ANY);
    mainPanel->SetMinSize(this->GetSize());

    mainSizer = new wxBoxSizer(wxHORIZONTAL);
    mainPanel->SetSizer(mainSizer);

    leftPanel = new wxPanel(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    mainSizer->Add(leftPanel, 3, wxEXPAND);

    leftSizer = new wxBoxSizer(wxVERTICAL);
    leftPanel->SetSizer(leftSizer);

    rightPanel = new wxPanel(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    mainSizer->Add(rightPanel, 1, wxEXPAND);

    rightSizer = new wxBoxSizer(wxVERTICAL);
    rightPanel->SetSizer(rightSizer);

    choiceBox = new wxChoice(rightPanel, wxID_ANY);
    rightSizer->Add(choiceBox, 0, wxALL, 10);

    wxString choices[3];
    choices[0] = wxT("PREP");
    choices[1] = wxT("AI_RECOGNITION");
    choices[2] = wxT("AI_GENERATION");
    choiceBox->Append(3, choices);
    choiceBox->SetSelection(0);

    switchPhase(PREP);

    choiceBox->Bind(wxEVT_CHOICE, &mainFrameEventHandlers::onChoice, &mfehandlers);
}


void mainFrame::switchPhase(PHASE newPhase) {
    currentPhase = newPhase;

    leftPanel->DestroyChildren();
    leftSizer = new wxBoxSizer(wxVERTICAL);
    leftPanel->SetSizer(leftSizer);

    if (currentPhase == PREP) {
        spdlog::info("switchPhase: Switching to PREP phase");

        auto* PrepTitleText = new wxStaticText(leftPanel, wxID_ANY, wxT("PREP PHASE"));
        leftSizer->Add(PrepTitleText, 0, wxALL, 10);

        auto* button = new wxButton(leftPanel, wxID_ANY, wxT("Open file"));
        leftSizer->Add(button, 0, wxALL, 10);

        auto *leftSubPanel = new wxPanel(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
        leftSizer->Add(leftSubPanel, 1, wxEXPAND);

        auto *leftSubSizer = new wxBoxSizer(wxHORIZONTAL);
        leftSubPanel->SetSizer(leftSubSizer);

        auto* text = new wxStaticText(leftSubPanel, wxID_ANY, wxT("Select file to open: "));
        leftSubSizer->Add(text, 0, wxALL, 10);

        filePicker = new wxFilePickerCtrl(
            leftSubPanel,
            wxID_ANY,
            path,
            wxT("Select an image file"),
            wxT("*.png;*.jpg;*.jpeg;*.bmp;*.gif")
        );
        leftSubSizer->Add(filePicker, 0, wxALL, 10);

        errorText1 = new wxStaticText(leftPanel, wxID_ANY, wxT("Select file to open first!"));
        leftSizer->Add(errorText1, 0, wxALL, 10);
        errorText1->Hide();

        errorText2 = new wxStaticText(leftPanel, wxID_ANY, wxT("Close previous window!"));
        leftSizer->Add(errorText2, 0, wxALL, 10);
        errorText2->Hide();

        button->Bind(wxEVT_BUTTON, &mainFrameEventHandlers::openFileButtonHandler, &mfehandlers);
        filePicker->Bind(wxEVT_FILEPICKER_CHANGED, &mainFrameEventHandlers::openFilePickerHandler, &mfehandlers);

    }

    if (currentPhase == AI_RECOGNITION) {
        getImageSize();

        spdlog::info("switchPhase: Switching to AI_RECOGNITION phase");
        auto* AIRecognitionTitleText = new wxStaticText(leftPanel, wxID_ANY, wxT("AI RECOGNITION PHASE"));
        leftSizer->Add(AIRecognitionTitleText, 0, wxALL, 10);

        auto *leftSubPanel1 = new wxPanel(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
        leftSizer->Add(leftSubPanel1, 1, wxEXPAND);

        auto *leftSubSizer1 = new wxBoxSizer(wxHORIZONTAL);
        leftSubPanel1->SetSizer(leftSubSizer1);

        auto *leftSubPanel2 = new wxPanel(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
        leftSizer->Add(leftSubPanel2, 1, wxEXPAND);

        auto *leftSubSizer2 = new wxBoxSizer(wxHORIZONTAL);
        leftSubPanel2->SetSizer(leftSubSizer2);

        wxString tmpLabel1 = std::to_string(imageSizeX);
        wxString tmpLabel2 = std::to_string(imageSizeY);


        originalSizeX = new wxStaticText(leftSubPanel1, wxID_ANY, "New Size X: ", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
        leftSubSizer1->Add(originalSizeX, 0, wxALL, 10);

        sizeXSpinCtrl = new wxSpinCtrl(leftSubPanel1, wxID_ANY);
        sizeXSpinCtrl->SetValue(1920);
        sizeXSpinCtrl->SetRange(800,3840);                  //4:3 800x600 -> 16:9 4K - 3840x2160
        leftSubSizer1->Add(sizeXSpinCtrl, 0, wxALL, 10);

        originalSizeX = new wxStaticText(leftSubPanel1, wxID_ANY, "Original Image Size X: ", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
        leftSubSizer1->Add(originalSizeX, 0, wxALL, 10);


        originalSizeX = new wxStaticText(leftSubPanel1, wxID_ANY, tmpLabel1, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
        leftSubSizer1->Add(originalSizeX, 0, wxALL, 10);


        originalSizeY = new wxStaticText(leftSubPanel2, wxID_ANY, "New Size Y: ", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
        leftSubSizer2->Add(originalSizeY, 0, wxALL, 10);

        sizeYSpinCtrl = new wxSpinCtrl(leftSubPanel2, wxID_ANY);
        sizeYSpinCtrl->SetValue(1080);
        sizeYSpinCtrl->SetRange(600,2160);                  //4:3 800x600 -> 16:9 4K - 3840x2160
        leftSubSizer2->Add(sizeYSpinCtrl, 0, wxALL, 10);

        originalSizeX = new wxStaticText(leftSubPanel2, wxID_ANY, "Original Image Size Y: ", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
        leftSubSizer2->Add(originalSizeX, 0, wxALL, 10);

        originalSizeX = new wxStaticText(leftSubPanel2, wxID_ANY, tmpLabel2, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
        leftSubSizer2->Add(originalSizeX, 0, wxALL, 10);

        startRecognition = new wxButton(leftPanel, wxID_ANY, wxT("Start"));
        leftSizer->Add(startRecognition, 0, wxALL, 10);

        sizeXSpinCtrl->Bind(wxEVT_SPINCTRL, &mainFrameEventHandlers::onSizeXSpinCtrl, &mfehandlers);
        sizeYSpinCtrl->Bind(wxEVT_SPINCTRL, &mainFrameEventHandlers::onSizeYSpinCtrl, &mfehandlers);
        startRecognition->Bind(wxEVT_BUTTON, &mainFrameEventHandlers::startRecognition, &mfehandlers);
    }

    if (currentPhase == AI_GENERATION) {
        spdlog::info("switchPhase: Switching to AI_GENERATION phase");
        wxStaticText* AIGenerationTitleText = new wxStaticText(leftPanel, wxID_ANY, wxT("AI GENERATION PHASE"));
        leftSizer->Add(AIGenerationTitleText, 0, wxALL, 10);

        //TODO - AI_GENERATION PHASE mainFrame
    }

    leftSizer->Layout();
    mainSizer->Layout();
    mainPanel->Layout();
    Refresh();
}

void mainFrame::getImageSize() {
    std::string filePath = "../tmp/images/grayQuantized.jpg";
    cv::Mat img = cv::imread(filePath);
    if (img.empty()) {
        spdlog::error("getImageSize: Cannot read file {}", filePath);
    } else {
        imageSizeX = img.size[0];
        imageSizeY = img.size[1];
        spdlog::info("getImageSize: File '{}' size X: {}, Y: {}", filePath, imageSizeX, imageSizeY);
    }
}

mainFrame::~mainFrame() {
}

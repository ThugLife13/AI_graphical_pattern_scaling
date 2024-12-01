#include "../lib/mainFrame.h"

mainFrame::mainFrame(const wxString &title): wxFrame(nullptr, wxID_ANY, title, wxPoint(10,10), wxSize(470, 140)), mfehandlers(this) {
    wxPanel *mainPanel = new wxPanel(this, wxID_ANY);

    //opening new window (only if file picked)
    wxButton* button = new wxButton(mainPanel, wxID_ANY, wxT("Open file"),
                                         wxPoint(230, 60), wxSize(200, -1));

    wxStaticText* text = new wxStaticText(mainPanel, wxID_ANY, wxT("Select file to open: "),
                                                wxPoint(10, 10), wxSize(200, -1));

    errorText1 = new wxStaticText(mainPanel, wxID_ANY, wxT("Select file to open first!"), wxPoint(10, 60), wxSize(200, -1));
    errorText1->Hide();

    errorText2 = new wxStaticText(mainPanel, wxID_ANY, wxT("Close previous window!"), wxPoint(10, 60), wxSize(200, -1));
    errorText2->Hide();

    //file panel
    filePicker = new wxFilePickerCtrl(
        mainPanel,                                              // Parent panel or window
        wxID_ANY,                                               // ID (use wxID_ANY for default)
        path,                                                   // Default file path
        wxT("Select an image file"),                         // Message to show in the control
        wxT("*.png;*.jpg;*.jpeg;*.bmp;*.gif"),               // Wildcard for file types
        wxPoint(230, 10),                                 // Position (x, y)
        wxSize(200, -1),                                  // Size (width, height)
        wxFLP_DEFAULT_STYLE                                     // Style (use wxFLP_DEFAULT_STYLE for default behavior)
        );


    //*******************************************************************************************************
    //handlers
    button->Bind(wxEVT_BUTTON, &mainFrameEventHandlers::openFileButtonHandler, &mfehandlers);
    filePicker->Bind(wxEVT_FILEPICKER_CHANGED, &mainFrameEventHandlers::openFilePickerHandler, &mfehandlers);
}

mainFrame::~mainFrame() {
}

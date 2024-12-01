#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "mainFrameEventHandlers.h"
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/filepicker.h>

class mainFrame : public wxFrame {
public:
    mainFrame(const wxString &title);
    ~mainFrame();

    //path
    wxString path = " ";

    wxStaticText* errorText1;
    wxStaticText* errorText2;
    wxFilePickerCtrl* filePicker;

private:
    mainFrameEventHandlers mfehandlers;
};



#endif //MAINFRAME_H

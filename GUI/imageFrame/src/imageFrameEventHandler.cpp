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


    //if created
    if (newCore->generate(frame->originalFilePath)) {
        spdlog::info("onButtonClicked: Images generated. Showing radioBox");
        frame->radioBox->Show();
    }

}

void imageFrameEventHandler::onRadioBoxChanged(wxCommandEvent& evt) {
    if (!imageDisplay || !imagePanel) return;

    int selection = evt.GetSelection();
    wxImage newImage;

    switch (selection) {
        case 0: {
            newImage = defaultImage;
            spdlog::info("onRadioBoxChanged: Default image");
            break;
        }
        case 1: {
            wxString radioPath1 = wxStandardPaths::Get().GetExecutablePath();
            radioPath1 = wxPathOnly(radioPath1) + "\\..\\tmp\\images\\straight.jpg";
            newImage.LoadFile(radioPath1);
            spdlog::info("onRadioBoxChanged: Radio 1 Clicked: {}", radioPath1.ToStdString());
            break;
        }
        case 2: {
            wxString radioPath2 = wxStandardPaths::Get().GetExecutablePath();
            radioPath2 = wxPathOnly(radioPath2) + "\\..\\tmp\\images\\gray.jpg";
            newImage.LoadFile(radioPath2);
            spdlog::info("onRadioBoxChanged: Radio 2 Clicked: {}", radioPath2.ToStdString());
            break;
        }
        case 3: {
            wxString radioPath2 = wxStandardPaths::Get().GetExecutablePath();
            radioPath2 = wxPathOnly(radioPath2) + "\\..\\tmp\\images\\vertices_2.jpg";
            newImage.LoadFile(radioPath2);
            spdlog::info("onRadioBoxChanged: Radio 3 Clicked: {}", radioPath2.ToStdString());
            break;
        }
        case 4: {
            wxString radioPath3 = wxStandardPaths::Get().GetExecutablePath();
            radioPath3 = wxPathOnly(radioPath3) + "\\..\\tmp\\images\\vertices_3.jpg";
            newImage.LoadFile(radioPath3);
            spdlog::info("onRadioBoxChanged: Radio 4 Clicked: {}", radioPath3.ToStdString());
            break;
        }
        case 5: {
            wxString radioPath3 = wxStandardPaths::Get().GetExecutablePath();
            radioPath3 = wxPathOnly(radioPath3) + "\\..\\tmp\\images\\vertices_4.jpg";
            newImage.LoadFile(radioPath3);
            spdlog::info("onRadioBoxChanged: Radio 5 Clicked: {}", radioPath3.ToStdString());
            break;
        }
        case 6: {
            wxString radioPath3 = wxStandardPaths::Get().GetExecutablePath();
            radioPath3 = wxPathOnly(radioPath3) + "\\..\\tmp\\images\\combined.jpg";
            newImage.LoadFile(radioPath3);
            spdlog::info("onRadioBoxChanged: Radio 6 Clicked: {}", radioPath3.ToStdString());
            break;
        }
        default: {
            spdlog::info("onRadioBoxChanged: Out of bound");
            return;
        }
    }

    if (newImage.IsOk()) {
        currentImage = newImage;
        wxBitmap bitmap(newImage.Scale(imagePanel->GetSize().x, imagePanel->GetSize().y, wxIMAGE_QUALITY_HIGH));
        imageDisplay->SetBitmap(bitmap);
        sizeCorrection(imagePanel->GetSize());
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

#include "../lib/setup.h"

bool setup::OnInit() {
    mainFrame* MainFrame = new mainFrame("AI Graphical Pattern Scaling");
    MainFrame->Center();
    MainFrame->Maximize(false);
    MainFrame->Show(true);
    SetTopWindow(MainFrame);
    return true;
}

setup::setup() {
}

setup::~setup() {
}
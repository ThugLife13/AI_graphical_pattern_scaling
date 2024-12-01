#ifndef CORE_H
#define CORE_H

#include <wx/wx.h>
#include "mainFrame.h"

class setup : public wxApp {
public:
    bool OnInit();

    setup();

    ~setup();
};



#endif //CORE_H

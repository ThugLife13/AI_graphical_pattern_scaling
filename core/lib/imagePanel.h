#ifndef IMAGEPANEL_H
#define IMAGEPANEL_H

#include <wx/wx.h>
#include <wx/scrolwin.h>

class decompositionFrame;
class imagePanel : public wxPanel {
public:
    imagePanel(wxWindow* parent, decompositionFrame* frame);
    void setBitmap(const wxBitmap& bitmap, double zoom);
    wxRect getSelection() const;
    void clearSelection();

    wxBitmap getBitmap() const;

private:
    void onPaint(wxPaintEvent& event);
    void onMouseDown(wxMouseEvent& event);
    void onMouseUp(wxMouseEvent& event);
    void onMouseMove(wxMouseEvent& event);

    wxSize m_originalSize;

    wxPoint GetUnscrolledPosition(const wxPoint& pos) const;

    decompositionFrame* m_frame;
    wxBitmap m_bitmap;
    double m_zoom;
    wxPoint m_dragStart;
    wxRect m_selection;
    bool m_isDragging;
};

#endif
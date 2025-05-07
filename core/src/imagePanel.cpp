#include "../lib/imagePanel.h"
#include "../../GUI/decompositionFrame/lib/decompositionFrame.h"

imagePanel::imagePanel(wxWindow* parent, decompositionFrame* frame)
    : wxPanel(parent), m_frame(frame), m_zoom(1.0), m_isDragging(false) {
    Bind(wxEVT_PAINT, &imagePanel::onPaint, this);
    Bind(wxEVT_LEFT_DOWN, &imagePanel::onMouseDown, this);
    Bind(wxEVT_LEFT_UP, &imagePanel::onMouseUp, this);
    Bind(wxEVT_MOTION, &imagePanel::onMouseMove, this);
    m_bitmap = wxNullBitmap;
}

void imagePanel::setBitmap(const wxBitmap& bitmap, double zoom) {
    m_bitmap = bitmap;
    m_zoom = zoom;

    m_originalSize = wxSize(
        static_cast<int>(bitmap.GetWidth() / zoom),
        static_cast<int>(bitmap.GetHeight() / zoom)
    );

    SetMinSize(wxSize(bitmap.GetWidth(), bitmap.GetHeight()));

    GetParent()->FitInside();

    Refresh();
}

wxRect imagePanel::getSelection() const {

    return m_selection;
}

void imagePanel::clearSelection() {
    m_selection = wxRect();
    Refresh();
}

// Modified rectangle normalization
void imagePanel::onPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    dc.DrawBitmap(m_bitmap, 0, 0);
    
    if (m_selection.width > 0 && m_selection.height > 0) {
        dc.SetPen(wxPen(*wxRED, 2));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(
            m_selection.x * m_zoom,
            m_selection.y * m_zoom,
            m_selection.width * m_zoom,
            m_selection.height * m_zoom
        );
    }
}

void imagePanel::onMouseDown(wxMouseEvent& event) {
    wxPoint pos = event.GetPosition();
    m_dragStart = pos;
    m_isDragging = true;
    CaptureMouse();
}

void imagePanel::onMouseUp(wxMouseEvent& event) {
    if (m_isDragging) {
        ReleaseMouse();
        m_isDragging = false;

        wxPoint endPos = event.GetPosition();

        int x1 = std::min(m_dragStart.x, endPos.x);
        int y1 = std::min(m_dragStart.y, endPos.y);
        int x2 = std::max(m_dragStart.x, endPos.x);
        int y2 = std::max(m_dragStart.y, endPos.y);

        // Convert to original coordinates
        m_selection = wxRect(
            x1 / m_zoom, y1 / m_zoom,
            (x2 - x1) / m_zoom,
            (y2 - y1) / m_zoom
        );

        // Clamp to image bounds
        wxSize imgSize = m_bitmap.GetSize();
        m_selection = m_selection.Intersect(wxRect(0, 0, m_originalSize.x, m_originalSize.y));
        Refresh();
    }
}

void imagePanel::onMouseMove(wxMouseEvent& event) {
    if (m_isDragging) {
        wxPoint currentPos = event.GetPosition();
        
        // Manual rectangle normalization
        int x1 = std::min(m_dragStart.x, currentPos.x);
        int y1 = std::min(m_dragStart.y, currentPos.y);
        int x2 = std::max(m_dragStart.x, currentPos.x);
        int y2 = std::max(m_dragStart.y, currentPos.y);

        // Convert to original coordinates
        wxRect currentRect(
            x1 / m_zoom, y1 / m_zoom,
            (x2 - x1) / m_zoom,
            (y2 - y1) / m_zoom
        );

        // Clamp to image bounds
        wxSize imgSize = m_bitmap.GetSize();
        m_selection = currentRect.Intersect(wxRect(0, 0, m_originalSize.x, m_originalSize.y));
        Refresh();
    }
}

wxBitmap imagePanel::getBitmap() const {
    return m_bitmap;
}

wxPoint imagePanel::GetUnscrolledPosition(const wxPoint& pos) const {
    wxScrolledWindow* parent = dynamic_cast<wxScrolledWindow*>(GetParent());
    if (parent) {
        wxPoint unscrolledPos;
        parent->CalcUnscrolledPosition(pos.x, pos.y, &unscrolledPos.x, &unscrolledPos.y);
        return unscrolledPos;
    }
    return pos;
}
#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include <algorithm>
#include <cmath>

namespace UI {
struct Point { float x, y; Point(float x = 0, float y = 0) : x(x), y(y) {} };
struct Rect {
    float x, y, w, h;
    Rect(float x = 0, float y = 0, float w = 0, float h = 0) : x(x), y(y), w(w), h(h) {}
    bool contains(Point p) const { return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h; }
};

class Layout {
    int outputW, outputH;
    float zoom;
public:
    Layout(int w, int h, float scale = 1) : outputW(std::max(1, w)), outputH(std::max(1, h)),
        zoom(std::max(0.75f, std::min(1.5f, scale))) {}
    float width() const { return outputW / zoom; }
    float height() const { return outputH / zoom; }
    float scale() const { return zoom; }
    Rect pixels(Rect r) const {
        // Round shared edges, not sizes, so adjacent controls cannot develop gaps.
        float x = std::floor(r.x * zoom + 0.5f), y = std::floor(r.y * zoom + 0.5f);
        return Rect(x, y, std::floor((r.x + r.w) * zoom + 0.5f) - x,
            std::floor((r.y + r.h) * zoom + 0.5f) - y);
    }
    Point fromWindow(Point p, int windowW, int windowH) const {
        return Point(p.x * outputW / std::max(1, windowW) / zoom,
            p.y * outputH / std::max(1, windowH) / zoom);
    }
    Rect content(float maxWidth, float margin) const {
        float w = std::max(0.0f, std::min(maxWidth, width() - margin * 2));
        return Rect((width() - w) / 2, margin, w, std::max(0.0f, height() - margin * 2));
    }
    Rect row(Rect area, int index, float rowHeight, float gap) const {
        return Rect(area.x, area.y + index * (rowHeight + gap), area.w, rowHeight);
    }
};
}
#endif

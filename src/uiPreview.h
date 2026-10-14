#ifndef UI_PREVIEW_H
#define UI_PREVIEW_H
#include "uiRenderer.h"

namespace UI {
class Preview {
public:
    int focus;
    float volume;
    bool fullscreen;
    bool continued;
    Rect controls[3];
    Preview() : focus(0), volume(0.7f), fullscreen(false), continued(false) {}
    bool draw(Renderer &ui);
    void activate(int control, Point position);
};
}
#endif

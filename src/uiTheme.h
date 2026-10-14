#ifndef UI_THEME_H
#define UI_THEME_H

#include <SDL2/SDL.h>

namespace UI {
enum TextRole { Title, Body, Small, TextRoleCount };
enum State { Normal, Focused, Disabled };
struct Theme {
    SDL_Color background, surface, text, muted, accent, border, disabled;
    int fontSize[TextRoleCount];
    float margin, gap, padding, controlHeight;
    SDL_Color panel;
    float panelWidth, panelMargin, panelPadding, panelPaddingBottom;
    float buttonHeight, buttonGap, headingTop, headingHeight, headingGap, labelHeight;
    float panelMinTop, panelPosition, panelBottomMargin;
    float optionsWidth, optionsRowHeight, optionsRowGap;
    float modalWidth;
    float modalMessageGap;
    SDL_Color modalOverlay;
    unsigned int revision;
    Theme() : revision(1) {
#define UI_NUMBER(key, member, initial, minimum, maximum, integral) member = initial;
#define UI_COLOR(key, member, r, g, b, a) member = SDL_Color{r, g, b, a};
#include "uiTheme.def"
#undef UI_NUMBER
#undef UI_COLOR
    }
};
}
#endif

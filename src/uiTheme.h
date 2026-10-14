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
    unsigned int revision;
    Theme() : background{18, 20, 23, 255}, surface{32, 36, 40, 255},
        text{239, 242, 244, 255}, muted{163, 174, 182, 255},
        accent{96, 221, 157, 255}, border{67, 77, 86, 255}, disabled{105, 116, 124, 255},
        margin(32), gap(16), padding(20), controlHeight(48), revision(1) {
        fontSize[Title] = 38; fontSize[Body] = 22; fontSize[Small] = 18;
    }
};
}
#endif

#ifndef UI_MENU_CURSOR_H
#define UI_MENU_CURSOR_H

#include <SDL2/SDL.h>

namespace UI {
class MenuCursor {
    int previous;
    MenuCursor(const MenuCursor &);
    MenuCursor &operator=(const MenuCursor &);
public:
    MenuCursor() : previous(SDL_ShowCursor(SDL_QUERY)) { SDL_ShowCursor(SDL_ENABLE); }
    ~MenuCursor() { if (previous >= 0) SDL_ShowCursor(previous); }
};
}
#endif

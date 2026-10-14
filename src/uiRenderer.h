#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include "uiLayout.h"
#include "uiTheme.h"
#include <SDL2/SDL_ttf.h>
#include <map>
#include <string>

namespace UI {
class Renderer {
    struct Text {
        SDL_Texture *texture;
        int w, h;
        Text() : texture(NULL), w(0), h(0) {}
    };
    SDL_Renderer *renderer;
    std::string fontPath;
    std::map<int, TTF_Font*> fonts;
    std::map<std::string, Text> cache;
    Text &getText(const std::string &value, TextRole role, SDL_Color color);
    Renderer(const Renderer &);
    Renderer &operator=(const Renderer &);
public:
    Theme theme;
    Layout layout;
    Renderer(SDL_Renderer *renderer, const char *fontPath);
    ~Renderer();
    void begin(int w, int h, float scale = 1);
    void clearCache();
    Point measure(const std::string &text, TextRole role = Body);
    bool text(const std::string &text, Rect bounds, TextRole role, SDL_Color color, bool centered = false);
    void fill(Rect bounds, SDL_Color color);
    void shade(Rect bounds, SDL_Color color);
    void outline(Rect bounds, SDL_Color color);
    bool button(const std::string &label, Rect bounds, State state);
    void slider(Rect bounds, float fraction, State state);
    void toggle(Rect bounds, bool checked, State state);
};
}
#endif

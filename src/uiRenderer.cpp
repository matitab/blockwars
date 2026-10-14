#include "uiRenderer.h"
#include "uiConfig.h"
#include <sstream>
#include <stdexcept>

namespace UI {
Renderer::Renderer(SDL_Renderer *target, const char *path) : renderer(target), fontPath(path), layout(1360, 768) {
    loadTheme(theme);
}
Renderer::~Renderer() {
    clearCache();
    for (std::map<int, TTF_Font*>::iterator it = fonts.begin(); it != fonts.end(); ++it) TTF_CloseFont(it->second);
}
void Renderer::clearCache() {
    for (std::map<std::string, Text>::iterator it = cache.begin(); it != cache.end(); ++it) SDL_DestroyTexture(it->second.texture);
    cache.clear();
}
void Renderer::begin(int w, int h, float scale) {
    layout = Layout(w, h, scale);
    // A UI pass uses output pixels; the caller restores its world transform afterwards.
    SDL_RenderSetLogicalSize(renderer, 0, 0);
    SDL_RenderSetViewport(renderer, NULL);
    SDL_RenderSetScale(renderer, 1, 1);
    SDL_RenderSetClipRect(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, theme.background.r, theme.background.g, theme.background.b, 255);
    SDL_RenderClear(renderer);
}
Renderer::Text &Renderer::getText(const std::string &value, TextRole role, SDL_Color color) {
    int size = std::max(14, int(std::floor(theme.fontSize[role] * layout.scale() + 0.5f)));
    std::ostringstream key;
    key << fontPath << ':' << size << ':' << int(color.r) << ':' << int(color.g) << ':' << int(color.b)
        << ':' << int(color.a) << ':' << theme.revision << ':' << value;
    std::map<std::string, Text>::iterator existing = cache.find(key.str());
    if (existing != cache.end()) return existing->second;
    if (cache.size() >= 256) clearCache();
    TTF_Font *font;
    if (fonts.find(size) == fonts.end()) {
        font = TTF_OpenFont(fontPath.c_str(), size);
        if (!font) throw std::runtime_error(TTF_GetError());
        fonts[size] = font;
    } else font = fonts[size];
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, value.empty() ? " " : value.c_str(), color);
    if (!surface) throw std::runtime_error(TTF_GetError());
    Text entry;
    entry.w = surface->w; entry.h = surface->h;
    entry.texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!entry.texture) throw std::runtime_error(SDL_GetError());
    SDL_SetTextureBlendMode(entry.texture, SDL_BLENDMODE_BLEND);
    cache[key.str()] = entry;
    return cache[key.str()];
}
Point Renderer::measure(const std::string &value, TextRole role) {
    Text &entry = getText(value, role, theme.text);
    return Point(entry.w / layout.scale(), entry.h / layout.scale());
}
bool Renderer::text(const std::string &value, Rect bounds, TextRole role, SDL_Color color, bool centered) {
    // Fit with an ellipsis; callers can use measure() to choose wrapping/stacking first.
    std::string shown = value;
    Text *entry = &getText(shown, role, color);
    Rect pixels = layout.pixels(bounds);
    bool fits = entry->w <= pixels.w && entry->h <= pixels.h;
    while (entry->w > pixels.w && !shown.empty()) {
        size_t end = shown.size() - 1;
        while (end > 0 && (static_cast<unsigned char>(shown[end]) & 0xc0) == 0x80) --end;
        shown.erase(end);
        entry = &getText(shown + "...", role, color);
    }
    SDL_Rect destination = {int(pixels.x), int(pixels.y + (pixels.h - entry->h) / 2), entry->w, entry->h};
    if (centered) destination.x += int((pixels.w - entry->w) / 2);
    SDL_Rect clip = {int(pixels.x), int(pixels.y), std::max(0, int(pixels.w)), std::max(0, int(pixels.h))};
    SDL_RenderSetClipRect(renderer, &clip);
    SDL_RenderCopy(renderer, entry->texture, NULL, &destination);
    SDL_RenderSetClipRect(renderer, NULL);
    return fits;
}
void Renderer::fill(Rect bounds, SDL_Color color) {
    Rect p = layout.pixels(bounds);
    SDL_Rect r = {int(p.x), int(p.y), int(p.w), int(p.h)};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &r);
}
void Renderer::outline(Rect bounds, SDL_Color color) {
    Rect p = layout.pixels(bounds);
    SDL_Rect r = {int(p.x), int(p.y), int(p.w), int(p.h)};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &r);
}
void Renderer::shade(Rect bounds, SDL_Color color) {
    SDL_BlendMode previous;
    SDL_GetRenderDrawBlendMode(renderer, &previous);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    fill(bounds, color);
    SDL_SetRenderDrawBlendMode(renderer, previous);
}
bool Renderer::button(const std::string &label, Rect bounds, State state) {
    fill(bounds, theme.surface);
    outline(bounds, state == Focused ? theme.accent : theme.border);
    Rect inner(bounds.x + theme.padding, bounds.y, std::max(0.0f, bounds.w - theme.padding * 2), bounds.h);
    return text(label, inner, Body, state == Disabled ? theme.disabled : theme.text, true);
}
void Renderer::slider(Rect bounds, float fraction, State state) {
    fraction = std::max(0.0f, std::min(1.0f, fraction));
    Rect track(bounds.x, bounds.y + bounds.h / 2 - 2, bounds.w, 4);
    fill(track, theme.border);
    fill(Rect(track.x, track.y, track.w * fraction, track.h), state == Disabled ? theme.disabled : theme.accent);
    fill(Rect(bounds.x + (bounds.w - 12) * fraction, bounds.y + bounds.h / 2 - 10, 12, 20),
        state == Disabled ? theme.disabled : theme.text);
}
void Renderer::toggle(Rect bounds, bool checked, State state) {
    Rect box(bounds.x, bounds.y + (bounds.h - 24) / 2, 24, 24);
    outline(box, state == Disabled ? theme.disabled : theme.accent);
    if (checked) fill(Rect(box.x + 5, box.y + 5, 14, 14), state == Disabled ? theme.disabled : theme.accent);
}
}

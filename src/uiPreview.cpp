#include "uiPreview.h"
#include <SDL2/SDL_image.h>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace UI {
bool Preview::draw(Renderer &ui) {
    Rect area = ui.layout.content(960, ui.theme.margin);
    bool fits = ui.text("Blob Wars", Rect(area.x, area.y, area.w, 56), Title, ui.theme.text);
    fits &= ui.text("Ajustes", Rect(area.x, area.y + 64, area.w, 32), Small, ui.theme.muted);
    bool stacked = area.w < 760;
    float rowHeight = stacked ? 92 : 72;
    Rect body(area.x, area.y + 112, area.w, area.h - 112);
    const char *labels[] = {"Partida", "Volumen de m\xc3\xbasica", "Pantalla completa"};
    for (int i = 0; i < 3; ++i) {
        Rect row = ui.layout.row(body, i, rowHeight, ui.theme.gap);
        Rect label(row.x, row.y, stacked ? row.w : row.w * 0.45f, stacked ? 32 : row.h);
        controls[i] = stacked ? Rect(row.x, row.y + 40, row.w, ui.theme.controlHeight) :
            Rect(row.x + row.w * 0.5f, row.y + (row.h - ui.theme.controlHeight) / 2, row.w * 0.5f, ui.theme.controlHeight);
        fits &= ui.text(labels[i], label, Body, ui.theme.text);
        if (i == 0) {
            Rect disabled(controls[i].x + controls[i].w * 0.55f + 8, controls[i].y,
                controls[i].w * 0.45f - 8, controls[i].h);
            controls[i].w = controls[i].w * 0.55f - 8;
            fits &= ui.button(continued ? "Seleccionada" : "Continuar", controls[i], focus == i ? Focused : Normal);
            fits &= ui.button("Cargar", disabled, Disabled);
        }
        if (i == 1) {
            ui.slider(Rect(controls[i].x, controls[i].y, controls[i].w - 72, controls[i].h), volume, Normal);
            char value[16]; std::snprintf(value, sizeof value, "%d%%", int(volume * 100 + 0.5f));
            fits &= ui.text(value, Rect(controls[i].x + controls[i].w - 64, controls[i].y, 64, controls[i].h), Small, ui.theme.muted, true);
        }
        if (i == 2) {
            ui.toggle(controls[i], fullscreen, Normal);
            fits &= ui.text(fullscreen ? "Activada" : "Desactivada", Rect(controls[i].x + 40, controls[i].y, controls[i].w - 40, controls[i].h), Body, ui.theme.text);
        }
        if (i > 0 && focus == i) ui.outline(controls[i], ui.theme.accent);
        fits &= row.y + row.h <= ui.layout.height() - ui.theme.margin;
    }
    return fits;
}
void Preview::activate(int control, Point p) {
    focus = control;
    if (control == 0) continued = !continued;
    if (control == 1) volume = std::max(0.0f, std::min(1.0f, (p.x - controls[1].x) / std::max(1.0f, controls[1].w - 72)));
    if (control == 2) fullscreen = !fullscreen;
}
}

static int verify(const char *directory) {
    const int sizes[][2] = {{1024, 768}, {1280, 720}, {1360, 768}, {1920, 1080}};
    const float scales[] = {0.75f, 1.0f, 1.25f, 1.5f};
    for (int i = 0; i < 4; ++i) for (int s = 0; s < 4; ++s) {
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, sizes[i][0], sizes[i][1], 32, SDL_PIXELFORMAT_ARGB8888);
        if (!surface) throw std::runtime_error(SDL_GetError());
        SDL_Renderer *target = SDL_CreateSoftwareRenderer(surface);
        if (!target) { SDL_FreeSurface(surface); throw std::runtime_error(SDL_GetError()); }
        {
            UI::Renderer ui(target, "data/vera.ttf");
            UI::Preview preview;
            ui.begin(sizes[i][0], sizes[i][1], scales[s]);
            if (!preview.draw(ui)) throw std::runtime_error("Preview text or controls overflow");
            UI::Point regular = ui.measure("Texto UTF-8: \xc3\xa1\xc3\xa9\xc3\xb1");
            ui.theme.fontSize[UI::Body] += 4;
            UI::Point larger = ui.measure("Texto UTF-8: \xc3\xa1\xc3\xa9\xc3\xb1");
            if (larger.y <= regular.y) throw std::runtime_error("Text cache did not invalidate size");
            ui.theme.fontSize[UI::Body] -= 4;
            if (ui.text("Texto largo con UTF-8: \xc3\xa1\xc3\xa9\xc3\xb1", UI::Rect(0, 0, 20, 32), UI::Body, ui.theme.text))
                throw std::runtime_error("Long text did not report truncation");
            ui.theme.revision++;
            preview.focus = 2;
            preview.activate(2, UI::Point());
            preview.focus = 0;
            preview.activate(0, UI::Point());
            ui.begin(sizes[i][0], sizes[i][1], scales[s]);
            if (!preview.draw(ui)) throw std::runtime_error("Checked state overflow");
            SDL_RenderPresent(target);
            Uint32 background = SDL_MapRGB(surface->format, ui.theme.background.r, ui.theme.background.g, ui.theme.background.b);
            int changed = 0, antialiased = 0;
            for (int y = 0; y < surface->h; ++y) for (int x = 0; x < surface->w; ++x) {
                Uint32 pixel = reinterpret_cast<Uint32*>(static_cast<unsigned char*>(surface->pixels) + y * surface->pitch)[x];
                if ((pixel & 0xffffff) != (background & 0xffffff)) ++changed;
                Uint8 r, g, b; SDL_GetRGB(pixel, surface->format, &r, &g, &b);
                bool solidMuted = r == ui.theme.muted.r && g == ui.theme.muted.g && b == ui.theme.muted.b;
                if (!solidMuted && r > 110 && r < 230 && g > 110 && g < 230 && b > 110 && b < 230) ++antialiased;
            }
            if (changed < 1000 || antialiased < 100) throw std::runtime_error("Blank frame or missing antialiased text");
            char path[1024];
            std::snprintf(path, sizeof path, "%s/ui-%dx%d-%d.png", directory, sizes[i][0], sizes[i][1], int(scales[s] * 100));
            if (IMG_SavePNG(surface, path) != 0) throw std::runtime_error(IMG_GetError());
            std::printf("PASS %dx%d scale %.2f: text fits, frame nonblank (%d pixels)\n", sizes[i][0], sizes[i][1], scales[s], changed);
        }
        SDL_DestroyRenderer(target); SDL_FreeSurface(surface);
    }
    return 0;
}

int main(int argc, char **argv) {
    bool verification = argc == 3 && std::string(argv[1]) == "--verify";
    if (SDL_Init(verification ? 0 : SDL_INIT_VIDEO) != 0 || TTF_Init() != 0) {
        std::fprintf(stderr, "SDL init failed: %s\n", SDL_GetError()); return 1;
    }
    int result = 0;
    try {
        if (verification) result = verify(argv[2]);
        else {
            SDL_Window *window = SDL_CreateWindow("Blob Wars - UI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1360, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
            if (!window) throw std::runtime_error(SDL_GetError());
            SDL_SetWindowMinimumSize(window, 1024, 720);
            SDL_Renderer *target = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
            if (!target) throw std::runtime_error(SDL_GetError());
            {
                UI::Renderer ui(target, "data/vera.ttf"); UI::Preview preview;
                bool running = true, dragging = false; float scale = 1;
                while (running) {
                    int w, h; SDL_GetRendererOutputSize(target, &w, &h);
                    ui.begin(w, h, scale); preview.draw(ui);
                    SDL_Event event;
                    while (SDL_PollEvent(&event)) {
                        if (event.type == SDL_QUIT) running = false;
                        if (event.type == SDL_KEYDOWN) {
                            SDL_Keycode key = event.key.keysym.sym;
                            if (key == SDLK_ESCAPE) running = false;
                            if (key == SDLK_TAB || key == SDLK_DOWN) preview.focus = (preview.focus + 1) % 3;
                            if (key == SDLK_UP) preview.focus = (preview.focus + 2) % 3;
                            if (preview.focus == 1 && key == SDLK_LEFT) preview.volume = std::max(0.0f, preview.volume - 0.05f);
                            if (preview.focus == 1 && key == SDLK_RIGHT) preview.volume = std::min(1.0f, preview.volume + 0.05f);
                            if (preview.focus == 2 && (key == SDLK_SPACE || key == SDLK_RETURN)) preview.activate(2, UI::Point());
                            if (preview.focus == 0 && (key == SDLK_SPACE || key == SDLK_RETURN)) preview.activate(0, UI::Point());
                            if (key == SDLK_EQUALS) scale = std::min(1.5f, scale + 0.25f);
                            if (key == SDLK_MINUS) scale = std::max(0.75f, scale - 0.25f);
                        }
                        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                            int ww, wh; SDL_GetWindowSize(window, &ww, &wh);
                            UI::Point p = ui.layout.fromWindow(UI::Point(event.button.x, event.button.y), ww, wh);
                            for (int i = 0; i < 3; ++i) if (preview.controls[i].contains(p)) {
                                preview.activate(i, p);
                                dragging = i == 1;
                            }
                        }
                        if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) dragging = false;
                        if (event.type == SDL_MOUSEMOTION && dragging) {
                            int ww, wh; SDL_GetWindowSize(window, &ww, &wh);
                            preview.activate(1, ui.layout.fromWindow(UI::Point(event.motion.x, event.motion.y), ww, wh));
                        }
                    }
                    SDL_GetRendererOutputSize(target, &w, &h);
                    ui.begin(w, h, scale);
                    preview.draw(ui); SDL_RenderPresent(target); SDL_Delay(16);
                }
            }
            SDL_DestroyRenderer(target); SDL_DestroyWindow(window);
        }
    } catch (const std::exception &error) {
        std::fprintf(stderr, "UI verification failed: %s\n", error.what()); result = 1;
    }
    TTF_Quit(); SDL_Quit(); return result;
}

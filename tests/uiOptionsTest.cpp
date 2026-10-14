#define SDL_MAIN_HANDLED
#include "../src/uiOptionsView.h"
#include "../src/uiMenuCursor.h"
#include <SDL2/SDL_image.h>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

static std::vector<UI::OptionItem> load(const char *path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error(path);
    std::vector<UI::OptionItem> items;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream fields(line);
        std::string type, name, group, options;
        UI::OptionItem item;
        int x, y;
        if (!(fields >> type) || type == "END") continue;
        if (!(fields >> name >> group >> std::quoted(item.label) >> std::quoted(options) >>
            x >> y >> item.minimum >> item.maximum)) throw std::runtime_error("Invalid fixture row");
        if (type == "LABEL") continue;
        if (type == "RADIO") {
            item.type = UI::OptionRadio;
            std::istringstream choices(options);
            std::string choice;
            while (std::getline(choices, choice, '|')) item.choices.push_back(choice);
        } else if (type == "SLIDER" || type == "SMOOTH_SLIDER") item.type = UI::OptionSlider;
        else if (type == "KEYBOARD" || type == "JOYPAD") {
            item.type = UI::OptionBinding;
            item.binding = type == "KEYBOARD" ? "Left Ctrl" : "Button #12";
        }
        item.value = item.minimum;
        items.push_back(item);
    }
    return items;
}

// Locate controls through the public input behavior, independent of layout math.
static UI::Point find(UI::OptionsView &view, int target, UI::Layout layout) {
    for (float y = 0; y < layout.height(); y += 6) for (float x = 0; x < layout.width(); x += 8) {
        UI::Point p(x, y);
        view.pointer(p, false);
        if (view.pointer(p, true).index == target) {
            view.pointer(p, false);
            return p;
        }
    }
    return UI::Point(-1, -1);
}

int main(int argc, char **argv) {
    SDL_SetMainReady();
    if (SDL_Init(0) || TTF_Init()) return 1;
    try {
        SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
        assert(SDL_InitSubSystem(SDL_INIT_VIDEO) == 0);
        SDL_ShowCursor(SDL_DISABLE);
        {
            UI::MenuCursor menu;
            assert(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
            {
                UI::MenuCursor submenu;
                assert(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
            }
            assert(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
        }
        assert(SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE);
        SDL_ShowCursor(SDL_ENABLE);
        { UI::MenuCursor menu; }
        assert(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
        const char *paths[] = {"data/optionWidgets", "data/keyboardWidgets", "data/joystickWidgets",
            "data/gameplayWidgets", "data/cheatWidgets", "data/es/optionWidgets", "data/es/keyboardWidgets",
            "data/es/joystickWidgets", "data/es/cheatWidgets"};
        const char *titles[] = {"Options", "Keyboard", "Joystick", "Gameplay", "Cheats",
            "Opciones", "Teclado", "Joystick", "Trucos"};
        const int sizes[][2] = {{1024, 768}, {1280, 720}, {1360, 768}, {1920, 1080}};
        const float scales[] = {1.0f, 1.5f};
        for (int screen = 0; screen < 9; ++screen) for (int size = 0; size < 4; ++size) for (int scale = 0; scale < 2; ++scale) {
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, sizes[size][0], sizes[size][1], 32, SDL_PIXELFORMAT_ARGB8888);
            assert(surface);
            SDL_Renderer *target = SDL_CreateSoftwareRenderer(surface);
            assert(target);
            {
                UI::Renderer ui(target, "data/vera.ttf");
                UI::OptionsView view;
                std::vector<UI::OptionItem> items = load(paths[screen]);
                assert(!items.empty());
                for (int focus = 0; focus < int(items.size()); ++focus) {
                    for (int i = 0; i < int(items.size()); ++i) items[i].focused = i == focus;
                    ui.begin(sizes[size][0], sizes[size][1], scales[scale]);
                    if (!view.draw(ui, titles[screen], items)) throw std::runtime_error(std::string(paths[screen]) + ": text overflow");
                    UI::Point point = find(view, focus, ui.layout);
                    if (point.x < 0) throw std::runtime_error(std::string(paths[screen]) + ": focused control unreachable");
                    UI::OptionAction action = view.pointer(point, true);
                    assert(action.index == focus);
                    assert(action.activate == (items[focus].type == UI::OptionBinding || items[focus].type == UI::OptionButton));
                    if (!action.activate) assert(action.value >= items[focus].minimum && action.value <= items[focus].maximum);
                    if (items[focus].type == UI::OptionRadio && items[focus].choices.size() == 2)
                        assert(action.value == items[focus].minimum + 1);
                    view.pointer(point, false);
                    if (focus == 0 && argc == 2) {
                        SDL_RenderPresent(target);
                        Uint32 background = SDL_MapRGB(surface->format, ui.theme.background.r, ui.theme.background.g, ui.theme.background.b);
                        int changed = 0;
                        for (int y = 0; y < surface->h; ++y) for (int x = 0; x < surface->w; ++x) {
                            Uint32 pixel = reinterpret_cast<Uint32*>(static_cast<unsigned char*>(surface->pixels) + y * surface->pitch)[x];
                            if ((pixel & 0xffffff) != (background & 0xffffff)) ++changed;
                        }
                        assert(changed > 1000);
                        char path[1024];
                        std::snprintf(path, sizeof path, "%s/options-%d-%dx%d-%d.png", argv[1], screen,
                            sizes[size][0], sizes[size][1], int(scales[scale] * 100));
                        assert(IMG_SavePNG(surface, path) == 0);
                    }
                }
                // A disabled control cannot produce an action, including after focus scrolling.
                items[0].enabled = false;
                for (size_t i = 0; i < items.size(); ++i) items[i].focused = i == 0;
                ui.begin(sizes[size][0], sizes[size][1], scales[scale]);
                assert(view.draw(ui, titles[screen], items));
                assert(find(view, 0, ui.layout).x < 0);
                assert(view.pointer(UI::Point(-100, -100), true).index == -1);
            }
            SDL_DestroyRenderer(target);
            SDL_FreeSurface(surface);
        }
        // Slider dragging clamps outside its track and stops on release.
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 1360, 768, 32, SDL_PIXELFORMAT_ARGB8888);
        SDL_Renderer *target = SDL_CreateSoftwareRenderer(surface);
        {
            UI::Renderer ui(target, "data/vera.ttf");
            UI::OptionsView view;
            std::vector<UI::OptionItem> items(1);
            items[0].type = UI::OptionSlider;
            items[0].label = "Sensitivity";
            items[0].minimum = 1;
            items[0].maximum = 320;
            ui.begin(1360, 768);
            assert(view.draw(ui, "Joystick", items));
            UI::Point point = find(view, 0, ui.layout);
            assert(point.x >= 0);
            view.pointer(point, true);
            assert(view.pointer(UI::Point(2000, point.y), true).value == 320);
            assert(view.pointer(UI::Point(-100, point.y), true).value == 1);
            view.pointer(point, false);
            assert(view.pointer(UI::Point(2000, point.y), true).index == -1);
            // Wheel scrolling works even when the keyboard focus remains unchanged.
            items.assign(20, UI::OptionItem());
            for (size_t i = 0; i < items.size(); ++i) items[i].label = "Setting";
            items[0].focused = true;
            ui.begin(1360, 768);
            assert(view.draw(ui, "Options", items));
            assert(find(view, 19, ui.layout).x < 0);
            view.pointer(UI::Point(-1, -1), false, -20);
            ui.begin(1360, 768);
            assert(view.draw(ui, "Options", items));
            assert(find(view, 19, ui.layout).x >= 0);
            // Enlarging the viewport resets an old offset once the complete list fits.
            ui.begin(1360, 1400);
            assert(view.draw(ui, "Options", items));
            assert(find(view, 0, ui.layout).x >= 0);
            view.pointer(UI::Point(-1, -1), false, -20);
            ui.begin(1360, 1400);
            assert(view.draw(ui, "Options", items));
            assert(find(view, 0, ui.layout).x >= 0);
            assert(find(view, 19, ui.layout).x >= 0);
            // The real main menu also ignores wheel input when it fits at 1360x768.
            items = load("data/optionWidgets");
            ui.begin(1360, 768);
            assert(view.draw(ui, "Options", items));
            view.pointer(UI::Point(-1, -1), false, -20);
            ui.begin(1360, 768);
            assert(view.draw(ui, "Options", items));
            assert(find(view, 0, ui.layout).x >= 0);
            assert(find(view, int(items.size()) - 1, ui.layout).x >= 0);
        }
        SDL_DestroyRenderer(target);
        SDL_FreeSurface(surface);
        std::puts("PASS options: 72 layouts, all focus targets reachable, disabled controls, mouse actions and slider dragging");
    } catch (const std::exception &error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
    TTF_Quit();
    SDL_Quit();
    return 0;
}

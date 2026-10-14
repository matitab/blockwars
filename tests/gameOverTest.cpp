// Include the implementation to exercise its internal setup/reset helpers with
// the real engine and mission state, without starting a playable game session.
#include "../src/game.cpp"
#include <cassert>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <map>

Audio audio;
Config config;
Engine engine;
Game game;
GameData gameData;
Graphics graphics;
Map map;
ReplayData replayData;
MedalServer medalServer;
Entity defEnemy[MAX_ENEMIES];
Entity defItem[MAX_ITEMS];
Entity player;
Weapon weapon[MAX_WEAPONS];

static void loadFixture(const char *path) {
    engine.deleteWidgets();
    std::ifstream file(path);
    assert(file);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream fields(line);
        std::string type, name, group, label, options;
        int x, y, minimum, maximum;
        if (!(fields >> type) || type == "END") continue;
        assert(fields >> name >> group >> std::quoted(label) >> std::quoted(options) >> x >> y >> minimum >> maximum);
        assert(type == "BUTTON" || type == "LABEL");
        Widget *widget = new Widget;
        widget->type = type == "LABEL" ? WG_LABEL : WG_BUTTON;
        widget->setProperties(name.c_str(), group.c_str(), label.c_str(), options.c_str(), x, y, minimum, maximum);
        engine.addWidget(widget);
    }
}

static void verifyMenuRendering(const char *directory)
{
    SDL_Surface *scene = IMG_Load("gfx/main/CircuitBoard.jpg");
    assert(scene);
    const char *languages[] = {"", "es/", "ca/", "de/", "da/", "nl/", "pl/", "sv/"};
    const char *groups[] = {"options", "warning", "restartconf", "escapeconf", "quitconf", "trainconf", "gameover"};
    const int sizes[][2] = {{1024, 768}, {1280, 720}, {1360, 768}, {1920, 1080}};
    const float scales[] = {1, 1.5f};
    int rendered = 0;
    for (int language = 0; language < 8; ++language) for (int group = 0; group < 7; ++group)
    {
        std::string path = std::string("data/") + languages[language] + (group == 6 ? "gameOverWidgets" : "inGameWidgets");
        loadFixture(path.c_str());
        std::map<std::string, int> values;
        engine.highlightedWidget = NULL;
        for (Widget *w = (Widget*)engine.widgetList.getHead()->next; w; w = (Widget*)w->next)
        {
            w->visible = strcmp(w->groupName, groups[group]) == 0;
            if (group == 0 && strcmp(w->name, "train") == 0) w->visible = false;
            if (w->type == WG_BUTTON)
            {
                w->value = &values[w->name];
                if (w->visible && !engine.highlightedWidget) engine.highlightedWidget = w;
            }
        }
        for (int size = 0; size < 4; ++size) for (int scale = 0; scale < 2; ++scale)
        {
            SDL_Surface *frame = SDL_CreateRGBSurfaceWithFormat(0, sizes[size][0], sizes[size][1], 32, SDL_PIXELFORMAT_ARGB8888);
            SDL_Renderer *target = SDL_CreateSoftwareRenderer(frame);
            assert(frame && target);
            {
                UI::Renderer ui(target, "data/vera.ttf");
                TitleMenuView view;
                SDL_Texture *background = SDL_CreateTextureFromSurface(target, scene);
                assert(background);
                ui.begin(sizes[size][0], sizes[size][1], scales[scale]);
                SDL_RenderCopy(target, background, NULL, NULL);
                ui.shade(UI::Rect(0, 0, ui.layout.width(), ui.layout.height()), ui.theme.modalOverlay);
                const char *heading = group == 6 ? "Game Over" : group == 0 ? "Paused" : "Confirm";
                if (!view.draw(ui, heading, true))
                {
                    SDL_RenderPresent(target);
                    if (directory) IMG_SavePNG(frame, (std::string(directory) + "/modal-failure.png").c_str());
                    std::ostringstream error;
                    error << path << ": modal overflow in " << groups[group] << " at " << sizes[size][0]
                        << "x" << sizes[size][1] << " scale " << scales[scale];
                    throw std::runtime_error(error.str());
                }
                SDL_RenderPresent(target);
                // Preserve the actual game asset under the dim layer.
                Uint8 originalR, originalG, originalB, shadedR, shadedG, shadedB;
                Uint32 original;
                Uint32 shaded = *static_cast<Uint32*>(frame->pixels);
                // IMG_Load may return 24-bit pixels; convert before reading the source.
                SDL_Surface *source32 = SDL_ConvertSurfaceFormat(scene, SDL_PIXELFORMAT_ARGB8888, 0);
                original = *static_cast<Uint32*>(source32->pixels);
                SDL_GetRGB(original, source32->format, &originalR, &originalG, &originalB);
                SDL_GetRGB(shaded, frame->format, &shadedR, &shadedG, &shadedB);
                assert(shadedR < originalR || shadedG < originalG || shadedB < originalB);
                assert(shadedR + shadedG + shadedB > 0);
                SDL_FreeSurface(source32);
                if (directory && language < 2 && size == 2 && scale == 0)
                {
                    char output[1024];
                    snprintf(output, sizeof output, "%s/modal-%d-%s.png", directory, language, groups[group]);
                    assert(IMG_SavePNG(frame, output) == 0);
                }
                // An unmoved mouse must not override keyboard focus.
                view.resetPointer(false);
                Widget *focus = engine.highlightedWidget;
                view.processPointer(680, 384, false);
                assert(engine.highlightedWidget == focus);
                SDL_DestroyTexture(background);
            }
            SDL_DestroyRenderer(target);
            SDL_FreeSurface(frame);
            ++rendered;
        }
    }
    SDL_FreeSurface(scene);
    printf("PASS modern modals: %d layouts, all confirmation groups, wrapped text and dimmed game asset\n", rendered);
}

int main(int argc, char **argv) {
    assert(SDL_Init(0) == 0);
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    assert(SDL_InitSubSystem(SDL_INIT_VIDEO) == 0);
    assert(TTF_Init() == 0);
    const char *files[] = {"data/gameOverWidgets", "data/es/gameOverWidgets", "data/ca/gameOverWidgets",
        "data/de/gameOverWidgets", "data/da/gameOverWidgets", "data/nl/gameOverWidgets",
        "data/pl/gameOverWidgets", "data/sv/gameOverWidgets"};
    const int counts[] = {-1, 0, 1, 3};
    for (const char *path : files) for (bool boss : {false, true}) for (int count : counts) {
        loadFixture(path);
        map.isBossMission = boss;
        game.canContinue = count;
        int cont = 0, restart = 0, quit = 0;
        configureGameOverMenu(cont, restart, quit);
        bool available = !boss && count > 0;
        assert(engine.getWidgetByName("gameOverNo")->enabled == available);
        assert(engine.getWidgetByName("gameOverRestart")->enabled);
        assert(cont == 0 && restart == 0 && quit == 0);
        assert(strcmp(engine.highlightedWidget->name, available ? "gameOverNo" : "gameOverRestart") == 0);
        // The default action must never quit automatically, even without a checkpoint.
        engine.clearInput();
        engine.keyState[SDL_SCANCODE_RETURN] = 1;
        engine.processWidgets();
        assert(quit == 0 && (available ? cont == 1 : restart == 1));
        cont = restart = quit = 0;
        engine.highlightWidget("gameOverRestart");
        engine.keyState[SDL_SCANCODE_RETURN] = 1;
        engine.processWidgets();
        assert(restart == 1 && cont == 0 && quit == 0);
    }
    game.setStageName("Current Stage");
    map.setName("Current Map");
    Data *current = new Data, *mia = new Data, *other = new Data;
    current->set("Current Stage", "Objective", 1, 1);
    mia->set("Current Stage MIAs", "MIAs", 1, 5);
    other->set("Other Stage", "Objective", 1, 1);
    gameData.dataList.add(current);
    gameData.dataList.add(mia);
    gameData.dataList.add(other);
    Persistant *currentMap = new Persistant, *otherMap = new Persistant;
    currentMap->setName("Current Map"); currentMap->addLine("test");
    otherMap->setName("Other Map"); otherMap->addLine("test");
    map.persistantList.add(currentMap);
    map.persistantList.add(otherMap);
    game.continueFromCheckPoint = true;
    game.canContinue = 2;
    restartGameOverLevel();
    assert(!game.continueFromCheckPoint && game.canContinue == 0);
    assert(gameData.dataList.getHead()->next == other && other->next == NULL);
    assert(strcmp(currentMap->stageName, "@none@") == 0 && currentMap->dataList.getHead()->next == NULL);
    assert(strcmp(otherMap->stageName, "Other Map") == 0 && otherMap->dataList.getHead()->next != NULL);
    assert(strcmp(game.stageName, "Current Stage") == 0);
    verifyMenuRendering(argc == 2 ? argv[1] : NULL);
    // Options can overwrite the software surface; leaving the modal restores it.
    graphics.registerEngine(&engine);
    graphics.window = SDL_CreateWindow("Menu test", 0, 0, 1360, 768, SDL_WINDOW_HIDDEN);
    graphics.renderer = SDL_CreateRenderer(graphics.window, -1, SDL_RENDERER_SOFTWARE);
    graphics.screen = SDL_CreateRGBSurfaceWithFormat(0, 800, 600, 32, SDL_PIXELFORMAT_ARGB8888);
    assert(graphics.window && graphics.renderer && graphics.screen);
    Uint32 color = SDL_MapRGB(graphics.screen->format, 80, 130, 90);
    SDL_FillRect(graphics.screen, NULL, color);
    SDL_Rect square = {80, 80, 60, 60};
    SDL_FillRect(graphics.screen, &square, SDL_MapRGB(graphics.screen->format, 200, 40, 30));
    SDL_ShowCursor(SDL_DISABLE);
    {
        GameMenuScreen screen("data/vera.ttf");
        assert(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
        SDL_FillRect(graphics.screen, NULL, 0);
        screen.draw("Paused");
        SDL_Surface *frame = SDL_CreateRGBSurfaceWithFormat(0, 1360, 768, 32, SDL_PIXELFORMAT_ARGB8888);
        assert(SDL_RenderReadPixels(graphics.renderer, NULL, frame->format->format, frame->pixels, frame->pitch) == 0);
        int minX = 1360, minY = 768, maxX = -1, maxY = -1;
        for (int y = 0; y < 200; ++y) for (int x = 0; x < 1360; ++x)
        {
            Uint32 pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(frame->pixels) + y * frame->pitch)[x];
            Uint8 red, green, blue;
            SDL_GetRGB(pixel, frame->format, &red, &green, &blue);
            if (red > green * 2 && red > blue * 2)
            {
                minX = std::min(minX, x); maxX = std::max(maxX, x);
                minY = std::min(minY, y); maxY = std::max(maxY, y);
            }
        }
        assert(maxX >= minX && maxY >= minY);
        assert(abs((maxX - minX) - (maxY - minY)) <= 1);
        assert(maxX - minX >= 75 && maxX - minX <= 78);
        assert((*static_cast<Uint32*>(frame->pixels) & 0xffffff) == 0);
        if (argc == 2) assert(IMG_SavePNG(frame, (std::string(argv[1]) + "/modal-preserved-4x3.png").c_str()) == 0);
        SDL_FreeSurface(frame);
    }
    assert(*static_cast<Uint32*>(graphics.screen->pixels) == color);
    assert(SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE);
    SDL_FreeSurface(graphics.screen); graphics.screen = NULL;
    SDL_DestroyRenderer(graphics.renderer); graphics.renderer = NULL;
    SDL_DestroyWindow(graphics.window); graphics.window = NULL;
    engine.deleteWidgets();
    TTF_Quit();
    SDL_Quit();
    std::puts("PASS Game Over: 64 checkpoint/boss/localization cases, explicit retry and isolated mission reset");
    return 0;
}

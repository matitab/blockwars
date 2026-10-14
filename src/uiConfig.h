#ifndef UI_CONFIG_H
#define UI_CONFIG_H

#include "uiTheme.h"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace UI {
inline bool readTheme(std::istream &file, Theme &theme, const char *path) {
    Theme candidate;
    bool valid = true;
    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        line.erase(line.find('#') == std::string::npos ? line.size() : line.find('#'));
        std::istringstream tokens(line);
        std::string key, equals, extra;
        if (!(tokens >> key)) continue;
        bool accepted = false;
        if ((tokens >> equals) && equals == "=") {
#define UI_NUMBER(name, member, initial, minimum, maximum, integral) \
            if (key == #name) { \
                double value; \
                if ((tokens >> value) && !(tokens >> extra) && std::isfinite(value) && \
                    value >= minimum && value <= maximum && (!integral || std::floor(value) == value)) { \
                    candidate.member = value; accepted = true; \
                } \
            }
#define UI_COLOR(name, member, r, g, b, a) \
            if (key == #name) { \
                int red, green, blue, alpha; \
                if ((tokens >> red >> green >> blue >> alpha) && !(tokens >> extra) && \
                    red >= 0 && red <= 255 && green >= 0 && green <= 255 && \
                    blue >= 0 && blue <= 255 && alpha >= 0 && alpha <= 255) { \
                    candidate.member = SDL_Color{Uint8(red), Uint8(green), Uint8(blue), Uint8(alpha)}; accepted = true; \
                } \
            }
#include "uiTheme.def"
#undef UI_NUMBER
#undef UI_COLOR
        }
        if (!accepted) {
            std::fprintf(stderr, "%s:%d: invalid UI setting: %s\n", path, lineNumber, key.c_str());
            valid = false;
        }
    }
    if (file.bad()) {
        theme = Theme();
        return false;
    }
    theme = candidate;
    return valid;
}

inline bool loadTheme(Theme &theme, const char *path = "ui.cfg") {
    // Loose config allows tuning without rebuilding either the game or its pak.
    std::ifstream file(path);
    if (!file) {
        theme = Theme();
        std::fprintf(stderr, "%s: unavailable; using default UI theme\n", path);
        return false;
    }
    return readTheme(file, theme, path);
}
}
#endif

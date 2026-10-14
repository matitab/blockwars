#define SDL_MAIN_HANDLED
#include "../src/uiConfig.h"
#include <cassert>
#include <sstream>

int main() {
    using namespace UI;
    UI::Theme defaults, loaded;
    assert(UI::loadTheme(loaded));
#define UI_NUMBER(key, member, initial, minimum, maximum, integral) assert(loaded.member == defaults.member);
#define UI_COLOR(key, member, redDefault, greenDefault, blueDefault, alphaDefault) \
    assert(loaded.member.r == defaults.member.r && loaded.member.g == defaults.member.g && \
        loaded.member.b == defaults.member.b && loaded.member.a == defaults.member.a);
#include "../src/uiTheme.def"
#undef UI_NUMBER
#undef UI_COLOR

    std::istringstream custom("# Comment\nPANEL_WIDTH = 520\nPANEL_PADDING_BOTTOM = 30 # inline\n"
        "FONT_BODY = 24\nACCENT_COLOR = 1 2 3 128\nPANEL_POSITION = 0.5\n");
    assert(UI::readTheme(custom, loaded, "custom"));
    assert(loaded.panelWidth == 520 && loaded.panelPaddingBottom == 30);
    assert(loaded.fontSize[UI::Body] == 24 && loaded.panelPosition == 0.5f);
    assert(loaded.accent.r == 1 && loaded.accent.a == 128);
    assert(loaded.buttonHeight == defaults.buttonHeight);

    std::istringstream invalid("FONT_BODY = 22.5\nBUTTON_HEIGHT = -1\nPANEL_POSITION = nan\n"
        "SURFACE_COLOR = 256 0 0 255\nTEXT_COLOR = 1 2 3\nUNKNOWN = 1\n"
        "PANEL_WIDTH = 500 extra\nBUTTON_GAP = 12\n");
    assert(!UI::readTheme(invalid, loaded, "invalid"));
    assert(loaded.fontSize[UI::Body] == defaults.fontSize[UI::Body]);
    assert(loaded.buttonHeight == defaults.buttonHeight && loaded.panelPosition == defaults.panelPosition);
    assert(loaded.surface.r == defaults.surface.r && loaded.text.r == defaults.text.r);
    assert(loaded.panelWidth == defaults.panelWidth && loaded.buttonGap == 12);
    assert(!UI::loadTheme(loaded, ".build/ui/nonexistent-ui.cfg"));
    assert(loaded.buttonGap == defaults.buttonGap);
    std::puts("PASS UI config: defaults, overrides, invalid values and missing file");
    return 0;
}

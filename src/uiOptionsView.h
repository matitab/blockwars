#ifndef UI_OPTIONS_VIEW_H
#define UI_OPTIONS_VIEW_H

#include "uiRenderer.h"
#include <vector>

namespace UI {
enum OptionType { OptionButton, OptionRadio, OptionSlider, OptionBinding };
struct OptionItem {
    std::string label, binding;
    std::vector<std::string> choices;
    OptionType type;
    int value, minimum, maximum;
    bool enabled, focused, capturing;
    OptionItem() : type(OptionButton), value(0), minimum(0), maximum(1),
        enabled(true), focused(false), capturing(false) {}
};
struct OptionAction {
    int index, value;
    bool activate;
    OptionAction(int index = -1, int value = 0, bool activate = false) :
        index(index), value(value), activate(activate) {}
};

class OptionsView {
    struct Hit {
        int index, choice;
        Rect rect;
        Hit(int index, int choice, Rect rect) : index(index), choice(choice), rect(rect) {}
    };
    std::vector<Hit> hits;
    std::vector<OptionItem> items;
    int firstRow, maxFirstRow, lastFocus, dragging;
    bool previousLeft;
public:
    OptionsView() : firstRow(0), maxFirstRow(0), lastFocus(-1), dragging(-1), previousLeft(false) {}
    bool draw(Renderer &ui, const std::string &title, const std::vector<OptionItem> &items);
    OptionAction pointer(Point position, bool left, int wheel = 0);
    void suspendPointer(bool left) { previousLeft = left; dragging = -1; }
};
}
#endif

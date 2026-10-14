#include "uiOptionsView.h"
#include <sstream>

namespace UI {
static std::vector<std::string> wrap(Renderer &ui, const std::string &text, float width) {
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word, line;
    while (words >> word) {
        std::string next = line.empty() ? word : line + " " + word;
        if (!line.empty() && ui.measure(next).x > width) {
            lines.push_back(line);
            line = word;
        } else line = next;
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

bool OptionsView::draw(Renderer &ui, const std::string &title, const std::vector<OptionItem> &model) {
    items = model;
    hits.clear();
    const Theme &theme = ui.theme;
    Rect area = ui.layout.content(theme.optionsWidth, theme.margin);
    float lineHeight = std::max(theme.labelHeight, ui.measure("Ag").y + 2);
    float headingHeight = std::max(theme.headingHeight, ui.measure(title, Title).y);
    bool fits = ui.text(title, Rect(area.x, area.y, area.w, headingHeight), Title, theme.text);
    float bodyTop = area.y + headingHeight + theme.gap;
    Rect body(area.x, bodyTop, area.w, std::max(0.0f, area.y + area.h - bodyTop));
    bool stacked = body.w < 700;
    float labelWidth = stacked ? body.w : body.w * 0.52f - theme.gap;
    float controlHeight = std::max(theme.optionsRowHeight, lineHeight + 8);
    std::vector<std::vector<std::string> > labels;
    std::vector<float> heights;
    int focus = -1;
    for (size_t i = 0; i < items.size(); ++i) {
        labels.push_back(wrap(ui, items[i].label, labelWidth));
        float labelHeight = labels.back().size() * lineHeight;
        heights.push_back(items[i].type == OptionButton ? controlHeight :
            (stacked ? labelHeight + theme.headingGap + controlHeight : std::max(controlHeight, labelHeight)));
        if (items[i].focused) focus = int(i);
    }
    // The last page fills the viewport; fitting lists never have a scroll offset.
    maxFirstRow = std::max(0, int(items.size()) - 1);
    float tailHeight = 0;
    for (int i = int(items.size()) - 1; i >= 0; --i) {
        float nextHeight = tailHeight + heights[i] + (tailHeight > 0 ? theme.optionsRowGap : 0);
        if (nextHeight > body.h) break;
        tailHeight = nextHeight;
        maxFirstRow = i;
    }
    firstRow = std::max(0, std::min(firstRow, maxFirstRow));
    // Keep keyboard/joystick focus visible without undoing mouse wheel scrolling.
    if (focus >= 0 && focus != lastFocus) {
        if (focus < firstRow) firstRow = focus;
        float height = 0;
        for (int i = firstRow; i <= focus; ++i) height += heights[i] + theme.optionsRowGap;
        while (height - theme.optionsRowGap > body.h && firstRow < focus) {
            height -= heights[firstRow++] + theme.optionsRowGap;
        }
    }
    firstRow = std::min(firstRow, maxFirstRow);
    lastFocus = focus;
    float y = body.y;
    int endRow = firstRow;
    for (int i = firstRow; i < int(items.size()); ++i) {
        const OptionItem &item = items[i];
        float height = heights[i];
        if (y + height > body.y + body.h) break;
        Rect row(body.x, y, body.w, height);
        Rect control = stacked ? Rect(row.x, y + height - controlHeight, row.w, controlHeight) :
            Rect(row.x + row.w * 0.52f, y + (height - controlHeight) / 2, row.w * 0.48f, controlHeight);
        State state = !item.enabled ? Disabled : item.focused ? Focused : Normal;
        SDL_Color color = item.enabled ? theme.text : theme.disabled;
        if (item.type == OptionButton) {
            control = Rect(row.x + row.w - std::min(theme.panelWidth, row.w), y,
                std::min(theme.panelWidth, row.w), controlHeight);
            fits &= ui.button(item.label, control, state);
            hits.push_back(Hit(i, -1, control));
        } else {
            float labelY = stacked ? y : y + (height - labels[i].size() * lineHeight) / 2;
            for (size_t l = 0; l < labels[i].size(); ++l)
                fits &= ui.text(labels[i][l], Rect(row.x, labelY + l * lineHeight, labelWidth, lineHeight), Body, color);
            if (item.type == OptionRadio) {
                int count = int(item.choices.size());
                if (count == 2 && item.maximum - item.minimum == 1) {
                    ui.fill(control, theme.surface);
                    ui.toggle(Rect(control.x + 12, control.y, 24, control.h), item.value != item.minimum, state);
                    int choice = item.value == item.minimum ? 0 : 1;
                    fits &= ui.text(item.choices[choice], Rect(control.x + 52, control.y, control.w - 64, control.h), Body, color);
                    hits.push_back(Hit(i, item.minimum + (choice == 0 ? 1 : 0), control));
                } else {
                    float width = control.w / std::max(1, count);
                    for (int c = 0; c < count; ++c) {
                        Rect segment(control.x + c * width, control.y, width, control.h);
                        ui.fill(segment, theme.surface);
                        ui.outline(segment, item.value == item.minimum + c && item.enabled ? theme.accent : theme.border);
                        fits &= ui.text(item.choices[c], Rect(segment.x + 4, segment.y, segment.w - 8, segment.h),
                            count > 2 ? Small : Body, color, true);
                        hits.push_back(Hit(i, item.minimum + c, segment));
                    }
                }
            } else if (item.type == OptionSlider) {
                Rect track(control.x + 8, control.y, std::max(1.0f, control.w - 80), control.h);
                float fraction = float(item.value - item.minimum) / std::max(1, item.maximum - item.minimum);
                ui.slider(track, fraction, state);
                std::ostringstream value; value << item.value;
                fits &= ui.text(value.str(), Rect(control.x + control.w - 64, control.y, 64, control.h), Small, color, true);
                hits.push_back(Hit(i, -1, track));
            } else {
                fits &= ui.button(item.capturing ? "..." : item.binding, control, state);
                hits.push_back(Hit(i, -1, control));
            }
            if (item.focused) ui.outline(control, theme.accent);
        }
        y += height + theme.optionsRowGap;
        endRow = i + 1;
    }
    if (firstRow > 0 || endRow < int(items.size())) {
        float barHeight = body.h * (endRow - firstRow) / std::max(1, int(items.size()));
        ui.fill(Rect(body.x + body.w + 8, body.y, 3, body.h), theme.border);
        ui.fill(Rect(body.x + body.w + 8, body.y + (body.h - barHeight) * firstRow /
            std::max(1, int(items.size()) - (endRow - firstRow)), 3, barHeight), theme.accent);
    }
    return fits;
}

OptionAction OptionsView::pointer(Point position, bool left, int wheel) {
    if (wheel) firstRow = std::max(0, std::min(maxFirstRow, firstRow - wheel));
    bool pressed = left && !previousLeft;
    previousLeft = left;
    if (!left) dragging = -1;
    for (size_t h = 0; h < hits.size(); ++h) {
        const Hit &hit = hits[h];
        const OptionItem &item = items[hit.index];
        if (!item.enabled) continue;
        bool inside = hit.rect.contains(position);
        if (item.type == OptionSlider && ((pressed && inside) || (left && dragging == hit.index))) {
            dragging = hit.index;
            float fraction = std::max(0.0f, std::min(1.0f, (position.x - hit.rect.x) / hit.rect.w));
            return OptionAction(hit.index, item.minimum + int(std::floor(fraction * (item.maximum - item.minimum) + 0.5f)));
        }
        if (pressed && inside) return OptionAction(hit.index, hit.choice, hit.choice < 0);
    }
    return OptionAction();
}
}

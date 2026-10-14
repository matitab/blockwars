#include "headers.h"
#include "titleMenuView.h"
#include <sstream>

extern Engine engine;

TitleMenuView::TitleMenuView() : previousMouseLeft(false), previousMouseX(-1), previousMouseY(-1) {}

const char *TitleMenuView::groupTitle() const
{
	Widget *highlight = engine.highlightedWidget;

	if ((highlight) && (strcmp(highlight->groupName, "skill") == 0))
		return _("Select Difficulty");

	if ((highlight) && (strcmp(highlight->groupName, "saveGames") == 0))
		return _("Load Game");

	if ((highlight) && (strcmp(highlight->groupName, "gameSlots") == 0))
		return _("Choose Save Slot");

	return _("Main Menu");
}

Widget *TitleMenuView::widgetAt(UI::Point position) const
{
	for (std::vector<Control>::const_iterator it = controls.begin(); it != controls.end(); ++it)
	{
		if ((it->widget->enabled) && (it->rect.contains(position)))
			return it->widget;
	}

	return NULL;
}

void TitleMenuView::activate(Widget *widget)
{
	if ((!widget) || (!widget->enabled) || (!widget->value))
		return;

	engine.highlightWidget(widget->name);
	*widget->value = 1;
	widget->changed = true;
}

int TitleMenuView::processPointer(int mouseX, int mouseY, bool mouseLeft)
{
	UI::Point position((float)mouseX, (float)mouseY);
	Widget *hovered = widgetAt(position);
	int sound = 0;
	bool moved = previousMouseX >= 0 && (previousMouseX != mouseX || previousMouseY != mouseY);
	bool pressed = mouseLeft && !previousMouseLeft;
	previousMouseX = mouseX;
	previousMouseY = mouseY;

	if ((moved || pressed) && (hovered) && (hovered != engine.highlightedWidget))
	{
		engine.highlightWidget(hovered->name);
		sound = 1;
	}

	if ((mouseLeft) && (!previousMouseLeft) && (hovered))
	{
		activate(hovered);
		sound = 2;
	}

	previousMouseLeft = mouseLeft;

	return sound;
}

static std::vector<std::string> menuLines(UI::Renderer &ui, const std::string &text, UI::TextRole role, float width)
{
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word, line;
    while (words >> word)
    {
        std::string next = line.empty() ? word : line + " " + word;
        if (!line.empty() && ui.measure(next, role).x > width)
        {
            lines.push_back(line);
            line = word;
        }
        else line = next;
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

bool TitleMenuView::draw(UI::Renderer &ui, const char *heading, bool centered)
{
    controls.clear();
    const UI::Theme &theme = ui.theme;
    UI::Rect area = ui.layout.content(centered ? theme.modalWidth : theme.panelWidth, theme.panelMargin);
    float innerWidth = std::max(0.0f, area.w - theme.panelPadding * 2);
    float textWidth = std::max(0.0f, innerWidth - theme.padding * 2);
    float bodyLine = std::max(theme.labelHeight, ui.measure("Ag", UI::Body).y);
    float smallLine = std::max(theme.labelHeight, ui.measure("Ag", UI::Small).y);
    std::vector<Widget*> buttons;
    std::vector<std::vector<std::string> > buttonLines, labels;
    std::vector<float> heights;
    float headerHeight = theme.headingTop + theme.headingHeight + theme.headingGap;
    float height = headerHeight + theme.panelPaddingBottom;

    for (Widget *w = (Widget*)engine.widgetList.getHead()->next; w; w = (Widget*)w->next)
    {
        if (!w->visible) continue;
        if (w->type == WG_LABEL)
        {
            labels.push_back(menuLines(ui, w->label, UI::Small, innerWidth));
            height += labels.back().size() * smallLine;
        }
        else if (w->type == WG_BUTTON && w->value)
        {
            buttons.push_back(w);
            buttonLines.push_back(menuLines(ui, w->label, UI::Body, textWidth));
            heights.push_back(std::max(theme.buttonHeight, buttonLines.back().size() * bodyLine + theme.headingGap));
            height += heights.back();
        }
    }
    height += std::max(0, int(buttons.size()) - 1) * theme.buttonGap;
    float messageGap = centered && !labels.empty() && !buttons.empty() ? theme.modalMessageGap : 0;
    height += messageGap;
    float top = centered ? (ui.layout.height() - height) / 2 :
        std::max(theme.panelMinTop, (ui.layout.height() - height) * theme.panelPosition);
    top = std::min(top, std::max(theme.panelMargin, ui.layout.height() - height - theme.panelBottomMargin));
    top = std::max(theme.panelMargin, top);
    UI::Rect panel(area.x, top, area.w, height);
    bool fits = panel.y + panel.h <= ui.layout.height() - theme.panelMargin;
    ui.fill(panel, theme.panel);
    float innerX = panel.x + theme.panelPadding;
    fits &= ui.text(heading ? heading : groupTitle(), UI::Rect(innerX, panel.y + theme.headingTop,
        innerWidth, theme.headingHeight), UI::Body, theme.text, true);
    float y = panel.y + headerHeight;
    for (size_t l = 0; l < labels.size(); ++l)
        for (size_t line = 0; line < labels[l].size(); ++line)
        {
            fits &= ui.text(labels[l][line], UI::Rect(innerX, y, innerWidth, smallLine), UI::Small, theme.muted, true);
            y += smallLine;
        }
    y += messageGap;
    for (size_t i = 0; i < buttons.size(); ++i)
    {
        Widget *w = buttons[i];
        UI::Rect rect(innerX, y, innerWidth, heights[i]);
        UI::State state = !w->enabled ? UI::Disabled : w == engine.highlightedWidget ? UI::Focused : UI::Normal;
        if (buttonLines[i].size() <= 1)
            fits &= ui.button(w->label, rect, state);
        else
        {
            ui.fill(rect, theme.surface);
            ui.outline(rect, state == UI::Focused ? theme.accent : theme.border);
            float textY = y + (rect.h - buttonLines[i].size() * bodyLine) / 2;
            for (size_t line = 0; line < buttonLines[i].size(); ++line)
                fits &= ui.text(buttonLines[i][line], UI::Rect(innerX + theme.padding, textY + line * bodyLine,
                    textWidth, bodyLine), UI::Body, state == UI::Disabled ? theme.disabled : theme.text, true);
        }
        Control control;
        control.widget = w;
        control.rect = rect;
        controls.push_back(control);
        y += rect.h + theme.buttonGap;
    }
    ui.outline(panel, theme.border);
    return fits;
}

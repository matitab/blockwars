#include "headers.h"
#include "titleMenuView.h"

extern Engine engine;

static SDL_Color rgba(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_Color color = {r, g, b, a};
	return color;
}

TitleMenuView::TitleMenuView() : previousMouseLeft(false) {}

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

	if ((hovered) && (hovered != engine.highlightedWidget))
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

void TitleMenuView::draw(UI::Renderer &ui)
{
	controls.clear();

	UI::Rect area = ui.layout.content(440, 28);
	float rowHeight = 40;
	float gap = 8;
	int visibleButtons = 0;
	int visibleLabels = 0;

	for (Widget *widget = (Widget*)engine.widgetList.getHead()->next; widget != NULL; widget = (Widget*)widget->next)
	{
		if (!widget->visible)
			continue;

		if (widget->type == WG_BUTTON)
			visibleButtons++;
		else if (widget->type == WG_LABEL)
			visibleLabels++;
	}

	float height = 72 + visibleButtons * rowHeight + max(0, visibleButtons - 1) * gap;
	height += visibleLabels * 24;
	float top = max(292.0f, (ui.layout.height() - height) * 0.74f);
	top = min(top, max(28.0f, ui.layout.height() - height - 42.0f));

	UI::Rect panel(area.x, top, area.w, height);
	ui.fill(panel, rgba(12, 14, 17, 215));

	ui.text(groupTitle(), UI::Rect(panel.x + 20, panel.y + 8, panel.w - 40, 36), UI::Body, ui.theme.text, true);

	float y = panel.y + 52;

	for (Widget *widget = (Widget*)engine.widgetList.getHead()->next; widget != NULL; widget = (Widget*)widget->next)
	{
		if ((!widget->visible) || (widget->type != WG_LABEL))
			continue;

		ui.text(widget->label, UI::Rect(panel.x + 20, y, panel.w - 40, 22), UI::Small, ui.theme.muted, true);
		y += 24;
	}

	for (Widget *widget = (Widget*)engine.widgetList.getHead()->next; widget != NULL; widget = (Widget*)widget->next)
	{
		if ((!widget->visible) || (widget->type != WG_BUTTON))
			continue;

		UI::Rect rect(panel.x + 20, y, panel.w - 40, rowHeight);
		UI::State state = widget->enabled ? UI::Normal : UI::Disabled;

		if (widget == engine.highlightedWidget)
			state = UI::Focused;

		ui.button(widget->label, rect, state);

		Control control;
		control.widget = widget;
		control.rect = rect;
		controls.push_back(control);

		y += rowHeight + gap;
	}

	ui.outline(panel, ui.theme.border);
}

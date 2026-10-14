#ifndef TITLE_MENU_VIEW_H
#define TITLE_MENU_VIEW_H

#include "uiRenderer.h"
#include <string>
#include <vector>

class Widget;

class TitleMenuView {
	struct Control {
		Widget *widget;
		UI::Rect rect;
	};

	std::vector<Control> controls;
	bool previousMouseLeft;
	int previousMouseX, previousMouseY;

	const char *groupTitle() const;
	Widget *widgetAt(UI::Point position) const;
	void activate(Widget *widget);

public:
	TitleMenuView();
	int processPointer(int mouseX, int mouseY, bool mouseLeft);
	bool draw(UI::Renderer &ui, const char *heading = NULL, bool centered = false);
	void resetPointer(bool mouseLeft) { controls.clear(); previousMouseLeft = mouseLeft; previousMouseX = previousMouseY = -1; }
};

#endif

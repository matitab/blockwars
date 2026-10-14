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

	const char *groupTitle() const;
	Widget *widgetAt(UI::Point position) const;
	void activate(Widget *widget);

public:
	TitleMenuView();
	int processPointer(int mouseX, int mouseY, bool mouseLeft);
	void draw(UI::Renderer &ui);
};

#endif

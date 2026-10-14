#include "../src/uiLayout.h"
#include <cassert>
#include <cstdio>

int main() {
    const int sizes[][2] = {{1024, 768}, {1280, 720}, {1360, 768}, {1920, 1080}};
    for (int i = 0; i < 4; ++i) {
        UI::Layout layout(sizes[i][0], sizes[i][1], 1.25f);
        UI::Rect content = layout.content(960, 32);
        assert(content.x >= 32 && content.x + content.w <= layout.width() - 32);
        UI::Rect first = layout.row(content, 0, 48, 16);
        UI::Rect second = layout.row(content, 1, 48, 16);
        assert(first.y + first.h < second.y);
        UI::Rect physical = layout.pixels(second);
        UI::Point mouse = layout.fromWindow(UI::Point((physical.x + physical.w / 2) / 2,
            (physical.y + physical.h / 2) / 2), sizes[i][0] / 2, sizes[i][1] / 2);
        assert(second.contains(mouse));
        assert(!second.contains(UI::Point(second.x + second.w, second.y)));
        UI::Rect left = layout.pixels(UI::Rect(0, 0, 33, 48));
        UI::Rect right = layout.pixels(UI::Rect(33, 0, 33, 48));
        assert(left.x + left.w == right.x);
        UI::Rect image = layout.fitImage(800, 600);
        assert(std::fabs(image.w / image.h - 4.0f / 3.0f) < 0.001f);
        assert(image.x >= 0 && image.y >= 0);
        assert(image.x + image.w <= layout.width() + 0.001f);
        assert(image.y + image.h <= layout.height() + 0.001f);
        assert(std::fabs(image.w - layout.width()) < 0.001f || std::fabs(image.h - layout.height()) < 0.001f);
    }
    UI::Layout invalid(0, -10, 0);
    assert(invalid.width() > 0 && invalid.height() > 0);
    assert(invalid.content(960, 32).w == 0);
    std::puts("UI layout tests passed");
}

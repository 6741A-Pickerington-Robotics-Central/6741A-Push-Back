#include "pros/screen.hpp"
class BrainScreen {
public:
    void draw_rect(int x1, int y1, int x2, int y2) {
        pros::screen::fill_rect(x1, y1, x2, y2);
    }
};

class Button {
private:
    int x, y, w, h;
    std::string label;
    uint32_t bgColor;
    uint32_t textColor;
    bool wasPressedLast = false;
    std::function<void()> onClick; // saved action

public:
    Button(int x, int y, int w, int h, std::string label,
           uint32_t bgColor = 0x303030, uint32_t textColor = 0xFFFFFF)
        : x(x), y(y), w(w), h(h), label(label),
          bgColor(bgColor), textColor(textColor) {}

    void setOnClick(std::function<void()> action) {
        onClick = action;
    }

    void draw() {
        pros::screen::set_pen(bgColor);
        pros::screen::fill_rect(x, y, x + w, y + h);
        pros::screen::set_pen(textColor);
        pros::screen::print_text(x + w/2, y + h/2, label.c_str(), pros::E_TEXT_ALIGN_CENTER);    }

    void update(const pros::screen_touch_status_s_t& touch) {
        bool pressed = false;
        if (touch.touch_status == 1) {
            if (touch.x >= x && touch.x <= x + w &&
                touch.y >= y && touch.y <= y + h) {
                pressed = true;
            }
        }

        // Detect click
        if (!pressed && wasPressedLast) {
            if (onClick) onClick();
        }

        wasPressedLast = pressed;
    }
};

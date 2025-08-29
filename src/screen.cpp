#include "pros/screen.hpp"
class BrainScreen {
public:
    void draw_rect(int x1, int y1, int x2, int y2) {
        pros::screen::fill_rect(x1, y1, x2, y2);
    }
};
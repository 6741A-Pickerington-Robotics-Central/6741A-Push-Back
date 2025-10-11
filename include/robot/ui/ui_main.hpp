// General headers
#include "pros/motors.h"
#include "pros/rtos.hpp"
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "liblvgl/lvgl.h"

// Include the main robot header for device access
#include "robot/robot.hpp"

// Include theUI headers
#include "robot/ui/commandbar.hpp"
#include "robot/ui/popup.hpp"

// Forward declarations of colors
extern lv_color_t purple;
extern lv_color_t yellow;
extern lv_color_t green;
extern lv_color_t blue;
extern lv_color_t red;
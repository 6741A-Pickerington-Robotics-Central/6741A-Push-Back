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
#include "robot/ui/commandbar.hpp"
#include "robot/robot.hpp"

// Optional forward declarations if you have them elsewhere
extern lv_color_t yellow;
extern lv_color_t green;
extern lv_color_t blue;
extern lv_color_t red;
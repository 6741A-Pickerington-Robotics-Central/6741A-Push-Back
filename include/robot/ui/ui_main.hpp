#pragma once
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
lv_color_t purple = lv_palette_main(LV_PALETTE_PURPLE);
lv_color_t red    = lv_palette_main(LV_PALETTE_RED);
lv_color_t green  = lv_palette_main(LV_PALETTE_GREEN);
lv_color_t blue   = lv_palette_main(LV_PALETTE_BLUE);
lv_color_t yellow = lv_palette_main(LV_PALETTE_YELLOW);

// Forward Function Declarations
void save_auton_event(lv_event_t* e);
lv_obj_t* create_number_button(lv_obj_t* parent, const char* text, int id);
static void goto_home(lv_event_t *e);
static void goto_editor(lv_event_t *e);
static void goto_diag(lv_event_t *e);

// Forward Declaration of random vars
std::string selected_auton = "";
const int TEMP_THRESHOLD = 50; // Temperature threshold for highlighting
lv_obj_t *btn_diag;
bool any_motor_over_temp = false;
// Track selection
static int selected_index = 0;
static int visible_offset = 0;
static const int VISIBLE_COUNT = 3; // number of visible containers
static lv_obj_t* list_inner;
static char number_input[16];        // current typed number (as string)
static lv_obj_t* active_button = NULL; // button that was clicked to open popup
lv_obj_t* display_label;
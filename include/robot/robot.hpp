#pragma once 
#include "pros/misc.hpp"
#include "liblvgl/lvgl.h"
#include <map>
#include "lemlib/api.hpp"
#include "api.h"
#include "main.h" // Include the main header for the PROS library

void Setup_lvgl_selector();

// Simple device list (you can expand this)
struct Device {
    std::string name;
    int port;
    bool is_motor;
    bool is_drive;
};

std::vector<Device> devices = {
    {"Left Front Drive Motor", 16, true, true},
    {"Left Middle Drive Motor", 18, true, true},
    {"Left Back Drive Motor", 19, true, true},
    {"Right Front Drive Motor", 15, true, true},
    {"Right Middle Drive Motor", 13, true, true},
    {"Right back Drive Motor", 11, true, true},
    {"Intake Motor 1", 3, true},
    {"Intake Motor 2", 10, true},
    {"Intake Motor 3", 8, true}
};
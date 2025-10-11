#ifndef ROBOT_HPP
#define ROBOT_HPP
#include "pros/misc.hpp"
#include "liblvgl/lvgl.h"
#include <map>
#include "lemlib/api.hpp"
#include "api.h"
#include "main.h" // Include the main header for the PROS library

void Setup_lvgl_selector();

//extern std::vector<Device> robotMotors;
extern lemlib::Chassis chassis;
extern pros::Controller controller;
extern pros::Motor Intake1;
extern pros::Motor Intake2;
extern pros::Motor Intake3;
extern pros::adi::DigitalOut weedwacker;
extern pros::adi::DigitalOut descore;

#endif // ROBOT_HPP
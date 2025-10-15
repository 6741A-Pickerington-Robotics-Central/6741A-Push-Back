#ifndef ROBOT_HPP
#define ROBOT_HPP
#include "pros/misc.hpp"
#include "liblvgl/lvgl.h"
#include <map>
#include "lemlib/api.hpp"
#include "api.h"
#include "main.h" // Include the main header for the PROS library
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

void Setup_lvgl_selector();
void skills_auton();
std::string get_auton_file_path();
void runtxtauton(const std::vector<std::string>& list);
std::vector<std::string> load_auton_for_runtxt(const std::string& filename);

//extern std::vector<Device> robotMotors;
extern lemlib::Chassis chassis;
extern pros::Controller controller;
extern pros::Motor Intake1;
extern pros::Motor Intake2;
extern pros::Motor Intake3;
extern pros::adi::DigitalOut weedwacker;
extern pros::adi::DigitalOut descore;
extern pros::adi::DigitalOut ballblock; // Blocks balls from exiting the intake

extern bool descoreState; // State of the weedwacker
extern int descoreCooldown; // Cooldown timer for weedwacker toggle
extern bool ballblockstate; // State of the ball block


// Selected auton vars
extern int selected_corner;  // 0=Red Left, 1=Red Right, 2=Blue Left, 3=Blue Right
extern int selected_slot;  // 0,1,2
extern bool autonrunning; // Flag to indicate if the auton is currently running

#endif // ROBOT_HPP
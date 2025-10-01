#include "pros/colors.hpp"
#include "pros/motors.h"
#include "pros/rtos.hpp"
#include "pros/screen.hpp"
#include "selector.h"
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "screen.cpp"
#include "liblvgl/lvgl.h"

const char* selected_auton = NULL;
const int TEMP_THRESHOLD = 50; // Temperature threshold for highlighting
lv_obj_t *btn_diag;
bool any_motor_over_temp = false;


/////////////////////////////////////////////
//             Run The Auton               //
/////////////////////////////////////////////
#if 0
void runauton(void) {
    if (running) return; // Prevent running multiple autons simultaneously
    running = true; // Set the running flag to true
    leftMotors.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE); // Set left motors to brake mode
    rightMotors.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE); // Set right motors to brake mode
    chassis.setPose(0, 0, 0); // Set position to x:0, y:0, heading:0
    if (index >= 0 && index < autonOptions.size()) {
        if (!autonOptions[index].loadFromFile()) {
            printf("Failed to load auton file!\n");
        } else {
            runtxtauton(autonOptions[index].getCommands());
        }
    } else if (selectedauton == -1) {
        skills_auton(); // Run the skills auton if selectedauton is -1
    } else if (selectedauton == -2) {
        pros::delay(1); //Do nothing
    }
    pros::delay(1000); // Allow IMU to stabilize
    leftMotors.set_brake_mode(pros::E_MOTOR_BRAKE_COAST); // Set left motors to coast mode
    rightMotors.set_brake_mode(pros::E_MOTOR_BRAKE_COAST); // Set right motors to coast mode
    running = false; // Set the running flag to false after the auton is complete
}
#endif

void runtxtauton(std::vector<std::string> list) {
    chassis.setPose(0, 0, 0); // Reset the chassis pose to (0, 0, 0) before running the auton
    if (list.empty()) {
      printf("The list is empty. No action taken.\n");
      return;
    }
    printf("Running a txt auton: %i\n", selectedautontoedit);
    // Iterate through the list
    for (const auto& item : list) {
      printf("Processing item: %s\n", item.c_str());  // Debugging line
      if (!item.empty()) {
        // Get the first letter of the current item
        char firstLetter = item[0];
        
        // Perform actions based on the first letter
        switch (firstLetter) {
            case 'm': {  // Handle 'm'
                printf("Processing '%s': First letter is 'm'. Performing move robot action.\n", item.c_str());
                const char* dataStr = item.c_str() + 1;  // Skip the first letter
            
                const char* firstComma = strchr(dataStr, ',');
                if (!firstComma) {
                    printf("Error: '%s' does not contain enough values.\n", item.c_str());
                    break;
                }
            
                const char* secondComma = strchr(firstComma + 1, ',');
                if (!secondComma) {
                    printf("Error: '%s' does not contain a third value.\n", item.c_str());
                    break;
                }
            
                char* endPtr;
                double x = std::strtod(dataStr, &endPtr);
                if (endPtr == dataStr || *endPtr != ',') {
                    printf("Error processing '%s': Could not extract a valid x.\n", item.c_str());
                    break;
                }
            
                double y = std::strtod(firstComma + 1, &endPtr);
                if (endPtr == firstComma + 1 || *endPtr != ',') {
                    printf("Error processing '%s': Could not extract a valid y.\n", item.c_str());
                    break;
                }
            
                double theta = std::strtod(secondComma + 1, &endPtr);
                if (endPtr == secondComma + 1) {
                    printf("Error processing '%s': Could not extract a valid theta.\n", item.c_str());
                    break;
                }
            
                printf("%f,%f,%f\n", x, y, theta);
                chassis.moveToPose(x, y, theta, 1000);
                break;
            }
            case 't': { // Handle 't'
            printf("Processing '%s': First letter is 't'. Performing turn robot action.\n", item.c_str());
            const char* dataStr = item.c_str() + 1;  // Skip the first letter
            double theta = std::strtod(dataStr, nullptr);  // Convert the remaining string to a double
            if (theta == 0.0 && dataStr[0] != '0') {
              printf("Error processing '%s': Could not convert to a valid theta.\n", item.c_str());
              break;
            }
            chassis.turnToHeading(theta, 1000); // Turn to the specified angle
            break;
            }  
            case 's': { // Handle 's'
                printf("Processing '%s': First letter is 's'. Performing swing robot action.\n", item.c_str());
                const char* dataStr = item.c_str() + 1;  // Skip the first letter
                double theta = std::strtod(dataStr, nullptr);  // Convert the remaining string to a double
                if (theta == 0.0 && dataStr[0] != '0') {
                  printf("Error processing '%s': Could not convert to a valid theta.\n", item.c_str());
                  break;
                }
                //chassis.swingToHeading(float theta, DriveSide lockedSide, int timeout); // Swing to the specified angle

            }
            default:  // Handle other cases
                ADIWrapper* matchingDevice = findADIByLetter(firstLetter);
                if (matchingDevice) {
                    printf("Processing '%s': Found matching ADIWrapper '%c'.\n", item.c_str(), firstLetter);
                    const char* numStr = item.c_str() + 1;  // Skip the first letter
                    float value = std::atof(numStr);  // Convert the remaining string to a float
                    if (value == 0.0 && numStr[0] != '0') {
                        printf("Error processing '%s': Could not convert to a valid float.\n", item.c_str());
                    } else {
                        matchingDevice->set_value(value != 0);
                    }
                } else {
                    printf("No matching ADIWrapper found for letter '%c'.\n", firstLetter);
                }
                MotorWrapper* matchingMotor = findMotorByLetter(firstLetter); // Search for a MotorWrapper with a matching letter      
                if (matchingMotor) {
                    printf("Processing '%s': Found matching MotorWrapper '%c'.\n", item.c_str(), firstLetter);

                    const char* numStr = item.c_str() + 1;  // Skip the first letter
                    int value = std::atoi(numStr);  // Convert the remaining string to an integer

                    if (value == 0 && numStr[0] != '0') {
                        printf("Error processing '%s': Could not convert to a valid integer.\n", item.c_str());
                    } else {
                        matchingMotor->set_velocity(value);
                    }
                } else {
                    printf("No matching MotorWrapper found for letter '%c'.\n", firstLetter);
                }
                break;
            }
      } else {
        printf("Skipping an empty item in the list.\n");
      }
    }
}  

/*

NEW AUTON SELECTOR USING LVGL

*/

// Page containers
lv_obj_t *page_home;
lv_obj_t *page_editor;
lv_obj_t *page_diag;
lv_obj_t *page_device;

// For device details
lv_obj_t *device_label_name;
lv_obj_t *device_label_port;
lv_obj_t *device_label_temp;
int current_device_port = -1;
bool current_device_is_motor = false;

// Simple device list (you can expand this)
struct Device {
    std::string name;
    int port;
    bool is_motor;
    bool is_drive;
};

std::vector<Device> devices = {
    {"Left Front Drive Motor", 1, true, true},
    {"Left Middle Drive Motor", 1, true, true},
    {"Left Back Drive Motor", 1, true, true},
    {"Right Front Drive Motor", 1, true, true},
    {"Right Middle Drive Motor", 1, true, true},
    {"Right back Drive Motor", 2, true, true},
    {"Intake Motor 1", 3, true},
    {"Intake Motor 2", 3, true},
    {"Intake Motor 3", 3, true},
    {"Intake Motor 4", 3, true}
};

// --- Utility: Show one page, hide others ---
void show_page(lv_obj_t *page) {
    lv_obj_add_flag(page_home, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_editor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_diag, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_device, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(page, LV_OBJ_FLAG_HIDDEN);
}

// --- Event callbacks ---
static void goto_home(lv_event_t *e) { show_page(page_home); }
static void goto_editor(lv_event_t *e) { show_page(page_editor); }
static void goto_diag(lv_event_t *e) { show_page(page_diag); }

void auton_btn_cb(lv_event_t * e) {
    lv_obj_t *btn = lv_event_get_target(e);       // the button that was pressed
    lv_obj_t *label = lv_obj_get_child(btn, 0);  // first child of button is usually the label
    selected_auton = lv_label_get_text(label);   // store the label text
    printf("Selected auton: %s\n", selected_auton);

    // Get the top bar label from button's user data
    lv_obj_t *top_label = (lv_obj_t *)lv_event_get_user_data(e);

    // Update the top bar label
    char buf[64];
    snprintf(buf, sizeof(buf), "Selected: %s", selected_auton);
    lv_label_set_text(top_label, buf);

    // Refresh display
    lv_obj_update_layout(top_label);
}

// Device click handler
static void goto_device(lv_event_t *e) {
    Device *dev = (Device *)lv_event_get_user_data(e);
    current_device_port = dev->port;
    current_device_is_motor = dev->is_motor;

    // Update labels
    lv_label_set_text_fmt(device_label_name, "Device: %s", dev->name.c_str());
    lv_label_set_text_fmt(device_label_port, "Port: %d", dev->port);

    if (dev->is_motor) {
        pros::Motor m(dev->port);
        int temp = m.get_temperature();
        lv_label_set_text_fmt(device_label_temp, "Temp: %d C", temp);
        lv_obj_clear_flag(device_label_temp, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(device_label_temp, LV_OBJ_FLAG_HIDDEN);
    }

    show_page(page_device);
}

// --- Page builders ---
void build_home_page() {
    page_home = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page_home, 480, 240);
    lv_obj_clear_flag(page_home, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(page_home, 0, 0);
    lv_obj_set_style_pad_all(page_home, 0, 0);
    lv_obj_set_style_bg_opa(page_home, LV_OPA_TRANSP, 0); // optional: transparent background

    // Top bar
    lv_obj_t *top_bar = lv_obj_create(page_home);
    lv_obj_set_size(top_bar, 480, 40);  // full width, 40px tall
    lv_obj_set_style_bg_color(top_bar, lv_color_white(), 0);
    lv_obj_set_pos(top_bar, 0, 0);
    lv_obj_clear_flag(top_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *auton_label = lv_label_create(top_bar);
    lv_label_set_text(auton_label, "Selected: None");
    lv_obj_set_style_text_color(auton_label, lv_color_black(), 0);
    lv_obj_center(auton_label);

    // Main area below top bar
    lv_obj_t *main_area = lv_obj_create(page_home);
    lv_obj_set_size(main_area, 480, 200);  // fills space under top bar
    lv_obj_set_pos(main_area, 0, 40);
    lv_obj_set_flex_flow(main_area, LV_FLEX_FLOW_ROW);  // horizontal split
    lv_obj_set_style_pad_all(main_area, 5, 0);
    lv_obj_set_style_bg_color(main_area, lv_color_hsv_to_rgb(280,255,255), 0);
    lv_obj_set_style_text_opa(main_area, LV_OPA_COVER, 0);

    // Left half with auton list
    lv_obj_t *auton_list = lv_list_create(main_area);
    lv_obj_set_size(auton_list, 220, LV_PCT(100));  // ~half width
    lv_obj_t *red_left_btn = lv_list_add_btn(auton_list, NULL, "Red Left");
    lv_obj_t *red_right_btn = lv_list_add_btn(auton_list, NULL, "Red Right");
    lv_obj_t *blue_left_btn = lv_list_add_btn(auton_list, NULL, "Blue Left");
    lv_obj_t *blue_right_btn = lv_list_add_btn(auton_list, NULL, "Blue Right");

    lv_obj_set_style_text_opa(lv_obj_get_child(red_left_btn, 0), LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(lv_obj_get_child(red_left_btn, 0), lv_color_hsv_to_rgb(10,255,255), 0);
    lv_obj_set_style_text_opa(lv_obj_get_child(red_right_btn, 0), LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(lv_obj_get_child(red_right_btn, 0), lv_color_hsv_to_rgb(10,255,255), 0); 
    lv_obj_set_style_text_opa(lv_obj_get_child(blue_left_btn, 0), LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(lv_obj_get_child(blue_left_btn, 0), lv_color_hsv_to_rgb(200,255,255), 0);
    lv_obj_set_style_text_opa(lv_obj_get_child(blue_right_btn, 0), LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(lv_obj_get_child(blue_right_btn, 0), lv_color_hsv_to_rgb(200,255,255), 0);

    lv_obj_add_event_cb(red_left_btn, auton_btn_cb, LV_EVENT_CLICKED, auton_label);
    lv_obj_add_event_cb(red_right_btn, auton_btn_cb, LV_EVENT_CLICKED, auton_label);
    lv_obj_add_event_cb(blue_left_btn, auton_btn_cb, LV_EVENT_CLICKED, auton_label);
    lv_obj_add_event_cb(blue_right_btn, auton_btn_cb, LV_EVENT_CLICKED, auton_label);


    // Right half with buttons
    lv_obj_t *button_col = lv_obj_create(main_area);
    lv_obj_set_size(button_col, 220, LV_PCT(100));
    lv_obj_clear_flag(button_col, LV_OBJ_FLAG_SCROLLABLE);

    btn_diag = lv_btn_create(button_col);
    lv_obj_t *lbl_diag = lv_label_create(btn_diag);
    lv_label_set_text(lbl_diag, "Diagnostics");
    lv_obj_align(lbl_diag, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(btn_diag, 0, 0);
    lv_obj_set_size(btn_diag, LV_PCT(100), 30);
    lv_obj_add_event_cb(btn_diag, goto_diag, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *btn_ports = lv_btn_create(button_col);
    lv_obj_t *lbl_ports = lv_label_create(btn_ports);
    lv_obj_align(lbl_ports, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(lbl_ports, "Port Reassign");
    lv_obj_set_pos(btn_ports, 0, 40);
    lv_obj_set_size(btn_ports, LV_PCT(100), 30);

    lv_obj_t *btn_editor = lv_btn_create(button_col);
    lv_obj_t *lbl_editor = lv_label_create(btn_editor);
    lv_obj_align(lbl_editor, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(lbl_editor, "Auton Editor");
    lv_obj_set_pos(btn_editor, 0, 80);
    lv_obj_set_size(btn_editor, LV_PCT(100), 30);
    lv_obj_add_event_cb(btn_editor, goto_editor, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_run = lv_btn_create(button_col);
    lv_obj_t *lbl_run = lv_label_create(btn_run);
    lv_obj_align(lbl_run, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(lbl_run, "Run");
    lv_obj_set_pos(btn_run, 0, 120);
    lv_obj_set_size(btn_run, LV_PCT(100), 30);
}

struct MotorLabelData {
    lv_obj_t *label;
    int port;
    int threshold;
    lv_color_t default_color;
    std::string name;
};

std::vector<MotorLabelData> motor_labels;

static void motor_blink_timer(lv_timer_t *timer) {
    any_motor_over_temp = false; // reset flag

    for (auto &data : motor_labels) {
        pros::Motor m(data.port);
        int temp = m.get_temperature();
        // Update label text
        if (!data.name.empty()) {
            lv_label_set_text_fmt(data.label, "%s - %dC", data.name.c_str(), temp);
        } else {
            lv_label_set_text_fmt(data.label, "%dC", temp);
        }
        // Blink if over threshold
        if (temp >= data.threshold) {
            any_motor_over_temp = true;
            lv_color_t current_color = lv_obj_get_style_text_color(data.label, 0);
            if(current_color.full == lv_color_hsv_to_rgb(0,255,255).full) {
                lv_obj_set_style_text_color(data.label, data.default_color, 0);
            } else {
                lv_obj_set_style_text_color(data.label, lv_color_hsv_to_rgb(0,255,255), 0);
            }
        } else {
            // Reset to default color if below threshold
            lv_obj_set_style_text_color(data.label, data.default_color, 0);
        }
    }
    // Update diagnostics button color
    if(!btn_diag) return;
    static bool toggle = false;
    if(any_motor_over_temp) {
        lv_color_t color = toggle ? lv_color_hsv_to_rgb(0,255,255) : lv_palette_main(LV_PALETTE_BLUE);
        lv_obj_set_style_bg_color(btn_diag, color, 0);
        toggle = !toggle;
    } else {
        lv_obj_set_style_bg_color(btn_diag, lv_palette_main(LV_PALETTE_BLUE), 0);
    }
}

void build_diag_page() {
    page_diag = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page_diag, 480, 240);
    lv_obj_clear_flag(page_diag, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(page_diag, 0, 0);
    lv_obj_set_style_pad_all(page_diag, 0, 0);
    lv_obj_set_style_bg_opa(page_diag, LV_OPA_TRANSP, 0); // optional: transparent background

    // Main horizontal container: left = motors, right = other items
    lv_obj_t *main_area = lv_obj_create(page_diag);
    lv_obj_set_size(main_area, 480, 240);
    lv_obj_set_pos(main_area, 0, 0);
    lv_obj_set_flex_flow(main_area, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(main_area, 5, 0);
    lv_obj_clear_flag(main_area, LV_OBJ_FLAG_SCROLLABLE);

    // Left side container: vertical stack for drivetrain label + motor boxes
    lv_obj_t *left_col = lv_obj_create(main_area);
    lv_obj_set_size(left_col, LV_PCT(40), LV_PCT(80)); // half width of main area
    lv_obj_set_flex_flow(left_col, LV_FLEX_FLOW_COLUMN); // stack children vertically
    lv_obj_set_style_pad_all(left_col, 0, 0);
    lv_obj_clear_flag(left_col, LV_OBJ_FLAG_SCROLLABLE);

    // Drivetrain label at top
    lv_obj_t *drivetrain_lbl = lv_label_create(left_col);
    lv_label_set_text(drivetrain_lbl, "Drivetrain Motors:");

    // Motor grid below label
    lv_obj_t *motor_grid = lv_obj_create(left_col);
    lv_obj_set_size(motor_grid, LV_PCT(100), LV_PCT(80)); // fill width of left_col
    lv_obj_set_flex_flow(motor_grid, LV_FLEX_FLOW_ROW_WRAP); // wrap items
    lv_obj_set_style_pad_all(motor_grid, 3, 0);
    lv_obj_set_style_pad_gap(motor_grid, 3, 0); // gap between boxes
    lv_obj_clear_flag(motor_grid, LV_OBJ_FLAG_SCROLLABLE);

    // Right side: other diagnostic items
    lv_obj_t *other_col = lv_obj_create(main_area);
    lv_obj_set_size(other_col, 240, LV_PCT(100));
    lv_obj_set_flex_flow(other_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(other_col, 5, 0);

    for (int i = 0; i < devices.size(); i++) {
        if (!devices[i].is_motor) continue;
        lv_obj_t *lbl;
        if(devices[i].is_drive) {
            lv_obj_t *motor_box = lv_obj_create(motor_grid);
            lv_obj_set_size(motor_box, LV_PCT(48), LV_PCT(33));
            lv_obj_clear_flag(motor_box, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(motor_box, lv_color_hsv_to_rgb(280,255,255), 0);
            lv_obj_set_style_pad_all(motor_box, 5, 0);
            lbl = lv_label_create(motor_box);
            lv_obj_center(lbl);
        } else {
            lbl = lv_label_create(other_col);
            lv_obj_center(lbl);
        }
        pros::Motor m(devices[i].port);
        int temp = m.get_temperature();
        // Initial text
        if(devices[i].is_drive) {
            lv_label_set_text_fmt(lbl, "%dC", temp);
        } else {
            lv_label_set_text_fmt(lbl, "%s - %dC", devices[i].name.c_str(), temp);
        }
        // Timer data
        MotorLabelData *data = new MotorLabelData;
        data->label = lbl;
        data->port = devices[i].port;
        data->threshold = TEMP_THRESHOLD;
        data->default_color = lv_color_white();
        if(!devices[i].is_drive) data->name = devices[i].name; // only set name for non-drive motors
        motor_labels.push_back(*data);
    }

    lv_timer_create(motor_blink_timer, 500, nullptr);

    // Back button
    lv_obj_t *btn_back = lv_btn_create(page_diag);
    lv_obj_set_size(btn_back, 190, 40);
    lv_obj_set_pos(btn_back, 5, 190);
    lv_obj_add_event_cb(btn_back, goto_home, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, "Back");
    lv_obj_center(lbl_back);
}

lv_obj_t *list_container;
std::vector<std::string> commandList;
lv_obj_t *keyboard;

static void ta_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(keyboard, ta);
        if (keyboard && lv_obj_is_valid(keyboard)) {
            lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        }
    }
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(keyboard, NULL);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

lv_obj_t* create_move_command_row(lv_obj_t* parent, double x, double y, double z) {
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_row(row, 5, 0);
    lv_obj_set_style_pad_column(row, 5, 0);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, "Move robot to:");

    // X, Y, Z textareas
    auto make_field = [&](const char *placeholder, double val) -> lv_obj_t* {
        lv_obj_t *ta = lv_textarea_create(row);
        lv_obj_set_width(ta, 50);
        lv_textarea_set_one_line(ta, true);
        lv_textarea_set_placeholder_text(ta, placeholder);

        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f", val);
        lv_textarea_set_text(ta, buf);
        lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, NULL);
        return ta;
    };

    make_field("x", x);
    make_field("y", y);
    make_field("z", z);

    return row;
}

// Callback for the "Add Command" button in the editor page
static void add_command_event(lv_event_t *e) {
    commandList.push_back("m0,0,0");

    // Create a new row in the container
    create_move_command_row(list_container, 0, 0, 0);

    // Optionally scroll to bottom so the new row is visible
    lv_obj_scroll_to_y(list_container, lv_obj_get_height(list_container), LV_ANIM_ON);
}



void add_keyboard(lv_obj_t *parent) {
    keyboard = lv_keyboard_create(parent);
    lv_obj_set_size(keyboard, 480, 120);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN); // Start hidden
}


void build_editor_page() {
    page_editor = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page_editor, 480, 240);
    lv_obj_clear_flag(page_editor, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(page_editor, 0, 0);
    lv_obj_set_style_pad_all(page_editor, 0, 0);
    lv_obj_set_style_bg_opa(page_editor, LV_OPA_TRANSP, 0);

    // Container for the list of commands
    list_container = lv_obj_create(page_editor);
    lv_obj_set_size(list_container, 395, 210);
    lv_obj_set_pos(list_container, 5, 5);
    lv_obj_set_flex_flow(list_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(list_container, LV_OBJ_FLAG_SCROLLABLE); // Turn off scroll

    // Add button
    lv_obj_t *btn_add = lv_btn_create(page_editor);
    lv_obj_set_size(btn_add, 70, 40);
    lv_obj_set_pos(btn_add, 405, 5);
    lv_obj_add_event_cb(btn_add, add_command_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_add = lv_label_create(btn_add);
    lv_label_set_text(lbl_add, "Add");
    lv_obj_center(lbl_add);

    // Remove button
    lv_obj_t *btn_remove = lv_btn_create(page_editor);
    lv_obj_set_size(btn_remove, 70, 40);
    lv_obj_set_pos(btn_remove, 405, 50);
    lv_obj_add_event_cb(btn_remove, add_command_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_remove = lv_label_create(btn_remove);
    lv_label_set_text(lbl_remove, "Del");
    lv_obj_center(lbl_remove);
    
    // Back button
    lv_obj_t *btn_back = lv_btn_create(page_editor);
    lv_obj_set_size(btn_back, 70, 30);
    lv_obj_set_pos(btn_back, 405, 200);
    lv_obj_add_event_cb(btn_back, goto_home, LV_EVENT_CLICKED, NULL);
    lv_label_set_text(lv_label_create(btn_back), "Home");
}

void build_device_page() {
    page_device = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page_device, 480, 240);
    lv_obj_clear_flag(page_device, LV_OBJ_FLAG_SCROLLABLE);

    device_label_name = lv_label_create(page_device);
    lv_obj_align(device_label_name, LV_ALIGN_TOP_LEFT, 10, 10);

    device_label_port = lv_label_create(page_device);
    lv_obj_align(device_label_port, LV_ALIGN_TOP_LEFT, 10, 40);

    device_label_temp = lv_label_create(page_device);
    lv_obj_align(device_label_temp, LV_ALIGN_TOP_LEFT, 10, 70);

    // Change port button
    lv_obj_t *btn_port = lv_btn_create(page_device);
    lv_obj_set_size(btn_port, 150, 40);
    lv_obj_align(btn_port, LV_ALIGN_TOP_LEFT, 10, 100);

    lv_obj_t *lbl_port = lv_label_create(btn_port);
    lv_label_set_text(lbl_port, "Change Port");
    lv_obj_center(lbl_port);

    // Back button
    lv_obj_t *btn_back = lv_btn_create(page_device);
    lv_obj_set_size(btn_back, 100, 40);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 10, 150);
    lv_obj_add_event_cb(btn_back, goto_diag, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, "Back");
    lv_obj_center(lbl_back);
}

void Setup_lvgl_selector() {
    build_home_page();
    build_editor_page();
    build_diag_page();
    build_device_page();

    // Start on auton page
    show_page(page_home);
}


/*

Old Auton Selector Code Below

*/

#if 0

void printListToScreen(const std::vector<std::string>& items, int start_y, int y_increment, int linehighlight, int shift_highlight) {
    int y = start_y; // Initial y-coordinate for printing
    if (linehighlight >= shift_highlight) y -= y_increment * (linehighlight - shift_highlight); // Adjust y-coordinate if linehighlight is too high
    if (debug) {
        set("pen","debug_secondary");
        pros::screen::draw_line(2, start_y, 20, 200); // Debugging line to visualize the area
        pros::screen::draw_line(20, start_y, 20, y_increment * items.size() + start_y); // Debugging line to visualize the area
    }
    set("fill", "bg main");
    set("pen", "text main");
    for (int i = 0; i < items.size(); i++) {
        if (i == linehighlight) set("fill","highlight"); // Set color for highlighted text
        else set("fill","bg main"); // Default color
        if (y >= start_y) pros::screen::print(pros::E_TEXT_MEDIUM, 2, y, items[i].c_str()); // Print text at specified position
        y += y_increment;
    }
    set("fill", "bg main");
    set("pen", "text main");
}

void printAutonNames(int startX, int startY, int yOffset, int selectedIndex, int currentID, int shift_highlight) {
    int y = startY; // Initial y-coordinate for printing
    if (selectedIndex >= shift_highlight) y -= yOffset * (selectedIndex - shift_highlight); // Adjust y-coordinate if linehighlight is too high
    if (debug) {
        set("pen","debug_secondary");
        pros::screen::draw_line(2, startY, 20, 200); // Debugging line to visualize the area
        pros::screen::draw_line(20, startY, 20, yOffset * autonOptions.size() + startY); // Debugging line to visualize the area
    }
    for (size_t i = 0; i < autonOptions.size(); i++) {
        set("pen", "text main"); // Reset pen color to white
        if (i == selectedIndex) {
            set("fill","highlight"); // Highlight selected auton
            if (i == currentID) set("pen", "highlight secondary"); // Highlight if it matches the current name
        } else {
            if (i == currentID) {
                set("pen", "debug secondary"); // Highlight if it matches the current name
                set("fill", "bg main"); // Default color
            } else {
                set("fill", "bg main"); // Default color
                set("pen", "text main"); // Reset pen color to white
            }
        }
        pros::screen::print(pros::E_TEXT_LARGE, startX, y, autonOptions[i].getName()); // Print the auton name
        y += yOffset;
    }
}

void selector() {
    loadautonsettingsFromFile(); // Load the auton settings from file
    update_screen(0); // Initial screen update
    while (true) {
      if (screencooldown <= 0) {
        status = pros::screen::touch_status(); // Get the current touch status
        switch (screen) {
            case 1: {
                if (button_press_at(0, 50, 200, 200,2)) { // Back
                    selectedauton -= 1;
                    check_for_loop();
                    update_screen(0);
                }
                if (button_press_at(200 , 50, 482, 200,2)) { // Forward
                    selectedauton += 1;
                    check_for_loop();
                    update_screen(0);
                }
                if (button_press_at(0, 200, 200, 272,1)) { // Run Auton
                    pros::delay(1000); // Wait for a second before running the auton
                    runauton();
                }
                if (button_press_at(200, 200, 480, 272,1)) { // Auton Editor
                    screen = 2;
                    update_screen(0);
                }
                if (screencooldown <= -300) { // Refresh the screen if it hasn't been updated for a while
                    screencooldown = 0;
                    update_screen(1);
                }
                break;
            }
            case 2: {
                if (button_press_at(380, 0, 480, 33,1)) { // Clear Selection
                    for (int i = 0; i < autonCountOld; i++) {
                        if (selectedautontoedit == autonOptions[i].getFileNumber()) {  // Compare with the index
                            autonOptions[i].setFileNumber(-1); // Clear the auton file number
                            break;  // Exit loop after clearing the name
                        }
                    }
                    saveautonsettingsToFile(); // Save the auton settings to file
                    update_screen(1);
                }
                if (button_press_at(160, 0, 315, 33,1)) { // Set Name
                    screen = 4; // Go to color selection screen
                    update_screen(0);
                }
                if (button_press_at(185, 120, 310, 170,1)) { // Load
                    loadtxtauton(generateFileName("A", selectedautontoedit));
                    screen = 3;
                    selectedline = 0;
                    update_screen(0);
                }
                if (button_press_at(0, 55, 160, 190,2)) { // Back arrow
                    selectedautontoedit -= 1;
                    update_screen(1);
                }
                if (button_press_at(320, 55, 480, 190,2)) { // Forward arrow
                    selectedautontoedit += 1;
                    update_screen(1);
                }
                if (button_press_at(0, 195, 480, 272,1)) { // Back to selector
                    screen = 1;
                    update_screen(0);
                }
                break;
            }
            case 3: { // Auton Editor
                if (saving == 2) {
                    if (save_timer <= 0) {
                        saving = 0;
                        update_screen(0);
                    } else {
                        save_timer -= 1;
                    }
                }
                if (button_press_at(400, 0, 480, 32,1)) { // Save
                    saving = 1;
                    save_timer = 5;
                    update_screen(1);
                    savetxtofauton(generateFileName("A", selectedautontoedit), items);
                    printf("Save\n");
                }
                if (button_press_at(300, 0, 380, 32,1)) { // Open
                    screen = 2;
                    update_screen(0);
                    printf("Open\n");
                }
                if (button_press_at(220, 0, 280, 32,1)) { // Run
                    printf("Run\n");
                    pros::delay(1000);
                    chassis.calibrate(); // calibrate sensors
                    runtxtauton(loadtxtauton(generateFileName("A", selectedautontoedit)));
                    update_screen(0);
                }
                // Scroll buttons
                if (button_press_at(0, 30, 120, 130,2)) {
                    selectedline -= 1;
                    update_screen(3);
                }
                if (button_press_at(0, 131, 120, 260,2)) {
                    selectedline += 1;
                    update_screen(3);
                }
                // Keyboard interactions
                if (button_press_at(125, 188, 185, 249,1)) { // Switch keyboard layout
                    keyboard = (keyboard % 3) + 1;
                    update_screen(2);
                }
                if (button_press_at(200, 188, 261, 249,1)) { // "-" minus sign
                    addCharToItem(items, selectedline, '-');
                    update_screen(3);
                }
                if (button_press_at(275, 188, 336, 249,1)) { // "." peirod
                    addCharToItem(items, selectedline, '.');
                    update_screen(3);
                }
                if (button_press_at(350, 188, 411, 249, 1)) { // "," comma
                    addCharToItem(items, selectedline, ',');
                    update_screen(3);
                }
                if (button_press_at(425, 188, 486, 249,1)) { // Remove last character/Backspace
                    removeLastCharacter(items, selectedline);
                    update_screen(3);
                }
                Key keyboardLayout1[] = {
                    {125, 35, '1'}, {200, 35, '2'}, {275, 35, '3'}, {350, 35, '4'}, {425, 35, '5'},
                    {125, 110, '6'}, {200, 110, '7'}, {275, 110, '8'}, {350, 110, '9'}, {425, 110, '0'}
                };
                Key keyboardLayout2[] = {
                    {125, 35, 'd'}, {200, 35, 't'}, {275, 35, 'w'}, {350, 35, 'm'}, {425, 35, ' '},
                    {125, 110, 'y'}, {200, 110, ' '}, {275, 110, ' '}, {350, 110, ' '}, {425, 110, 'E'}
                };
                Key keyboardLayout3[] = {
                    {125, 35, 'i'}, {200, 35, 'c'}, {275, 35, 'p'}, {350, 35, 'k'}, {425, 35, 'l'},
                    {125, 110, 'a'}, {200, 110, ' '}, {275, 110, ' '}, {350, 110, ' '}, {425, 110, 'E'}
                };
                Key* layout;
                switch (keyboard) {
                    case 1: layout = keyboardLayout1; break;
                    case 2: layout = keyboardLayout2; break;
                    case 3: layout = keyboardLayout3; updateKeyboardLayoutlayout(keyboardLayout3); break;
                }
                for (int i = 0; i < 10; i++) {
                    if (button_press_at(layout[i].x, layout[i].y, layout[i].x + 60, layout[i].y + 61,1)) { // Check if the button is pressed
                        if (layout[i].character == 'E') { // Enter key
                            insertNewLine(items, selectedline);
                            update_screen(3);
                        } else if (layout[i].character == ' ') { // Blank key
                            // Do nothing for blank key
                        } else if (layout[i].character != ' ') { // If it's not a blank key, add the character to the item
                            addCharToItem(items, selectedline, layout[i].character);
                            update_screen(3);
                        }
                    }
                }
                break;
            }
            case 4: {
                if (button_press_at(0, 20, 300, 131,2)) { //Scroll up
                    selectedName -= 1;
                    update_screen(1);
                }
                if (button_press_at(0, 132, 300, 272,2)) { //Scroll down
                    selectedName += 1;
                    update_screen(1);
                }
                if (button_press_at(300, 20, 480, 131,1)) { //Select color
                    autonOptions[selectedName].setFileNumber(selectedautontoedit);
                    saveautonsettingsToFile(); // Save the auton settings to file
                    screen = 2;
                    update_screen(0);
                }
                if (button_press_at(300, 132, 480, 272,1)) { //Back to auton editor
                    screen = 2;
                    update_screen(0);
                }
            }
        }
    }
    screencooldown -= 1;
    last_screen = screen; // Update last_screen to the current screen
    pros::delay(50);
    }
}

void update_screen(int update_mode) {
  //BrainScreen brainscreen;
  if (screenUpdating) return; 
  screenUpdating = true; // Prevent multiple updates at once
  switch (screen) {
    case 1: {
        if (update_mode == 0) {
          set("pen","text main");
          pros::screen::fill_rect(0, 0, 480, 272); // Background
          // Auton Bar
          set("fill","bg bar"); // Draw background bar
          //pros::screen::fill_rect(0, 0, 480, 50); // Fill the top bar with the background color
          //brainscreen.draw_rect(0, 0, 480, 50); // Draw a rectangle in the middle for the auton name
          set("pen","text bar"); // Set text color and print "Auton:"
          pros::screen::print(pros::E_TEXT_LARGE, 0, 10, "Auton:");
          if (selectedauton >= 0 && selectedauton < autonCountOld) { // Determine which auton is selected and print its name
              Auton selected = autonOptions[selectedauton]; // Get the selected auton object
              pros::screen::set_pen(selected.getColor()); // Set the pen color based on the auton color
              pros::screen::print(pros::E_TEXT_LARGE, 130, 10, selected.getName()); // Print the auton name at the specified position
          } else if (selectedauton == -1) {
              pros::screen::set_pen(pros::Color::black); // Default color
              pros::screen::print(pros::E_TEXT_LARGE, 130, 10, "Skills");
              sidecolor = 0;
          } else if (selectedauton == -2) {
              pros::screen::set_pen(pros::Color::black); // Default color
              pros::screen::print(pros::E_TEXT_LARGE, 130, 10, "None");
              sidecolor = 0;
          } else {
              pros::screen::set_pen(pros::Color::black); // Default color
              pros::screen::print(pros::E_TEXT_LARGE, 130, 10, "Invalid Auton");
              sidecolor = 0;
          }
          // Run/No Auton Button
          set("pen","bg main");
          pros::screen::fill_rect(0, 200, 480, 50);
        }
        if (update_mode == 1 || update_mode == 0) {
          set("pen","text main");
          // Temperatures   
          float averageleftdrivetemp = leftMotors.get_temperature();
          float averagerightdrivetemp = rightMotors.get_temperature();
          float row3Temp = temp3.get_temperature();
          float row4Temp = temp4.get_temperature();
          // Print temperatures to the screen
          set("pen","text main");
          set("fill","bg main");
          pros::screen::print(pros::E_TEXT_LARGE, 0, 60, "Left Motors: %.1f", averageleftdrivetemp);
          pros::screen::print(pros::E_TEXT_LARGE, 0, 95, "Right Motors: %.1f", averagerightdrivetemp);
          pros::screen::print(pros::E_TEXT_LARGE, 0, 130, (temp3name + std::string(" %.1f")).c_str(), row3Temp);
          pros::screen::print(pros::E_TEXT_LARGE, 0, 165, (temp4name + std::string(" %.1f")).c_str(), row4Temp);
        }
        if (update_mode == 0) {
          set("pen","text bar");
          set("fill","bg bar"); 
          pros::screen::fill_rect(205, 200, 215, 250);  
          if (selectedauton == 6 || selectedauton == 7) {
              pros::screen::print(pros::E_TEXT_LARGE_CENTER, 10, 210, "No Auton");
          } else {
              pros::screen::print(pros::E_TEXT_LARGE_CENTER, 10, 210, "Run");
          }
          pros::screen::print(pros::E_TEXT_LARGE_CENTER, 230, 210, "Auton Editor");
        }
        break;
    }
    case 2: {
        if (update_mode == 0) {
          set("pen","bg main");
          // Background
          pros::screen::fill_rect(0, 0, 480, 272);
          // Auton Editor UI
          set("pen","text main");
          set("fill","button");
          pros::screen::print(pros::E_TEXT_MEDIUM, 0, 1, "Auton Editor");
          pros::screen::print(pros::E_TEXT_LARGE, 380, 1, "Clear");
          pros::screen::print(pros::E_TEXT_LARGE, 160, 1, "Set Name");
        }
        set("fill","button");
        if (update_mode == 1 || update_mode == 0) {
          // File Name
          std::string filename = generateFileName("A", selectedautontoedit);
          pros::screen::print(pros::E_TEXT_LARGE, 175, 80, filename.c_str());
          set("pen","bg main");
          pros::screen::fill_rect(145, 45, 345, 80);
          set("pen","text main");
          set("fill","bg main");
          for (int i = 0; i < autonCountOld; i++) {
            if (selectedautontoedit == autonOptions[i].getFileNumber()) {  // Compare with the index
                pros::screen::set_pen(autonOptions[i].getColor());  // Set the pen color based on the auton color
                pros::screen::print(pros::E_TEXT_LARGE, 145, 45, autonOptions[i].getName());
                break;  // Exit loop after printing the name
            }
          }              
        }
        if (update_mode == 0) {
          // Draw UI Elements
          set("pen","text main");
          pros::screen::draw_line(145, 179, 80, 114);
          pros::screen::draw_line(145, 51, 80, 116);
          pros::screen::draw_line(345, 179, 410, 114);
          pros::screen::draw_line(345, 51, 410, 116);   
          // Load Button
          pros::screen::print(pros::E_TEXT_LARGE, 200, 130, "LOAD");
          pros::screen::draw_rect(185, 120, 310, 170);  
          // Back to Selector
          pros::screen::print(pros::E_TEXT_LARGE, 90, 200, "Back to Selector");
          pros::screen::draw_line(0, 195, 480, 195);
        }
        break;
    }
    case 3: {
        set("pen","bg main");
        if (update_mode == 0) {
            // Background
            pros::screen::fill_rect(0, 0, 480, 272);
        }
        if (update_mode == 0 || update_mode == 3) {
            // Auton Editor UI
            pros::screen::fill_rect(0, 33, 120, 272);
            printListToScreen(items, 40, 20, selectedline, 7); // Print the list of items to the screen
        }
        if (update_mode == 0) {
            set("pen","text main");
            set("fill","button");
            pros::screen::print(pros::E_TEXT_MEDIUM, 1,1, "Auton Editor");
        }
        set("fill","button");
        set("pen","text main");
        if (update_mode == 1 || update_mode == 0) {
            // Background color switch based on saving state
            switch (saving) {
                case 0: pros::screen::set_eraser(pros::Color::black); break;
                case 1: pros::screen::set_eraser(pros::Color::orange); break;
                case 2: pros::screen::set_eraser(pros::Color::green); break;
            }
            // Save, Open, Run buttons
            pros::screen::print(pros::E_TEXT_LARGE, 400, 1, "Save");
        }
        set("fill","button");
        if (update_mode == 0) {
            pros::screen::print(pros::E_TEXT_LARGE, 300, 1, "Open");
            pros::screen::print(pros::E_TEXT_LARGE, 220, 1, "Run");
            // File name
            std::string filename = generateFileName("A", selectedautontoedit);
            pros::screen::print(pros::E_TEXT_MEDIUM, 1,18, filename.c_str());
            // Other keys
            struct Key {
                int x, y;
                const char* label;
            } keys[] = {
                {350, 180, ","}, {275, 180, "."}, {200, 180, "-"}, {425, 180, "B"}
            };
            for (const auto& key : keys) {
                set("pen","button");
                pros::screen::fill_rect(key.x, key.y, key.x + 60, key.y + 61);
                set("pen","text main");
                pros::screen::draw_rect(key.x, key.y, key.x + 60, key.y + 61);
                set("fill","button");
                pros::screen::print(pros::E_TEXT_LARGE, key.x + 20, key.y + 15, key.label);
            }
        }
        if (update_mode == 0 || update_mode == 2) {
            KeyboardKey keyboard1[] = { //Numbers
                {125, 35, "1"}, {200, 35, "2"}, {275, 35, "3"}, {350, 35, "4"}, {425, 35, "5"},
                {125, 110, "6"}, {200, 110, "7"}, {275, 110, "8"}, {350, 110, "9"}, {425, 110, "0"}
            };
            KeyboardKey keyboard2[] = { //Movement letters
                {125, 35, "m"}, {200, 35, "t"}, {275, 35, "s"}, {350, 35, ""}, {425, 35, ""},
                {125, 110, "w"}, {200, 110, ""}, {275, 110, ""}, {350, 110, ""}, {425, 110, "E"}
            };
            KeyboardKey keyboard3[] = { //Other motor and ADI letters
                {125, 35, ""}, {200, 35, ""}, {275, 35, ""}, {350, 35, ""}, {425, 35, ""},
                {125, 110, ""}, {200, 110, ""}, {275, 110, ""}, {350, 110, ""}, {425, 110, ""}
            };
            KeyboardKey* layout;
            const char* switchLabel = "";
            switch (keyboard) {
                case 1: layout = keyboard1; switchLabel = "Mov"; break;
                case 2: layout = keyboard2; switchLabel = "Oth"; break;
                case 3: layout = keyboard3; switchLabel = "123"; updateKeyboardLayout(keyboard3); break;
            }
            // Draw keyboard switch button
            set("pen","button");
            set("fill","button");
            pros::screen::fill_rect(125, 180, 185, 249);
            set("pen","text main");
            pros::screen::draw_rect(125, 180, 185, 249);
            pros::screen::print(pros::E_TEXT_LARGE, 125, 195, switchLabel);
            for (int i = 0; i < 10; i++) { // Draw keyboard keys
                set("pen","button");
                set("fill","button");
                pros::screen::fill_rect(layout[i].x, layout[i].y, layout[i].x + 60, layout[i].y + 61);
                set("pen","text main");
                pros::screen::draw_rect(layout[i].x, layout[i].y, layout[i].x + 60, layout[i].y + 61);
                pros::screen::print(pros::E_TEXT_LARGE, layout[i].x + 20, layout[i].y + 15, layout[i].label);
            }
        }
        break;
    }          
    case 4: {
        if (update_mode == 0) {
            // Background
            set("pen","bg main");
            pros::screen::fill_rect(0, 0, 480, 272);
            // Auton Corrner Selector
            set("pen","text main");
            set("fill","button");
            pros::screen::print(pros::E_TEXT_MEDIUM, 0,1, "Auton Editor");
            pros::screen::draw_rect(301, 0, 480, 131);
            pros::screen::draw_rect(301, 132, 480, 272);
            pros::screen::print(pros::E_TEXT_LARGE, 310, 150, "Back to");
            pros::screen::print(pros::E_TEXT_LARGE, 310, 190, "Editor");
            pros::screen::print(pros::E_TEXT_LARGE, 310, 30, "Set");
            pros::screen::print(pros::E_TEXT_LARGE, 310, 70, "Name");
        }
        if (update_mode == 1 || update_mode == 0) {
            set("pen","bg main");
            pros::screen::fill_rect(0, 20, 300, 272);
            set("fill","bg main");
            printAutonNames(10, 20, 50, selectedName, getAutonIndexByFileID(selectedautontoedit), 3); // Print the auton names to the screen      
        }
    }  
  }
  screenUpdating = false;
}

#endif
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
#include "liblvgl/lvgl.h"

std::string selected_auton = "";
const int TEMP_THRESHOLD = 50; // Temperature threshold for highlighting
lv_obj_t *btn_diag;
bool any_motor_over_temp = false;


/////////////////////////////////////////////
//             Run The Auton               //
/////////////////////////////////////////////
void runauton(void);
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

// Page containers
lv_obj_t *page_home;
lv_obj_t *page_editor;
lv_obj_t *page_diag;

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

// --- Utility: Show one page, hide others ---
void show_page(lv_obj_t *page) {
    lv_obj_add_flag(page_home, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_editor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_diag, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(page, LV_OBJ_FLAG_HIDDEN);
}

// --- Event callbacks for switch pages ---
static void goto_home(lv_event_t *e) { show_page(page_home); }
static void goto_editor(lv_event_t *e) { show_page(page_editor); }
static void goto_diag(lv_event_t *e) { show_page(page_diag); }

void auton_btn_cb(lv_event_t * e) {
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    const char* lvgl_text = lv_label_get_text(label);
    selected_auton = lvgl_text;   // copies safely into std::string

    printf("Selected auton: %s\n", selected_auton.c_str());

    // Update top bar label
    lv_obj_t *top_label = (lv_obj_t *)lv_event_get_user_data(e);
    char buf[64];
    snprintf(buf, sizeof(buf), "Selected: %s", selected_auton.c_str());
    lv_label_set_text(top_label, buf);
    lv_obj_update_layout(top_label);
}

// --- Home Page ---
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

// --- Diagnostics Page ---
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

// --- Editor Page ---

/*
1. I do not like this code, need to change to my own keyboard implementation.
    Use buttons to make a keypad
    need keys 0-9 . and backspace
    Use a label to show the typed text

2. Make the screen actualy look good.
    Bars for code lines
    Add move up and down buttons
    Add selecttion controls

*/

lv_obj_t *list_container;
std::vector<std::string> commandList;
static lv_obj_t *keyboard = nullptr;
lv_obj_t *input_label;       // Shows typed characters
std::string input_buffer = ""; // Stores the current typed text

// Keymap rows, terminated by "" 
// LV_SYMBOLs (like LV_SYMBOL_BACKSPACE) are built-in icons
static const char * custom_kb_map[] = {
    "1", "2", "3", "\n",    // row 1
    "4", "5", "6", "\n",    // row 2
    "7", "8", "9", "\n",    // row 3
    ".", "0", LV_SYMBOL_BACKSPACE, "" // row 4 (then end)
};

// Optional: define control flags (makes some buttons wider, assigns styles)
static const lv_btnmatrix_ctrl_t custom_kb_ctrl[] = {
    1, 1, 1,                 // "1" "2" "3"
    1, 1, 1,                 // "4" "5" "6"
    1, 1, 1,                 // "7" "8" "9"
    1, 1, LV_BTNMATRIX_CTRL_CHECKED, // "." "0" "Del"
    LV_BTNMATRIX_CTRL_CHECKED,  // "Enter"
};

// Optional: handle keyboard's APPLY/READY key to hide it and defocus the textarea
static void keyboard_event_cb(lv_event_t *e) {
    lv_obj_t *kb = lv_event_get_target(e);
    const char *txt = lv_btnmatrix_get_btn_text(kb, lv_btnmatrix_get_selected_btn(kb));
    if (!txt) return;
    if (strcmp(txt, "Enter") == 0) {
        printf("Final input: %s\n", input_buffer.c_str());
        // You could store input_buffer somewhere here
        input_buffer.clear(); // clear after confirm
    }
    else if (strcmp(txt, "Del") == 0) {
        if (!input_buffer.empty())
            input_buffer.pop_back();
    }
    else { // normal character (0-9 or ".")
        input_buffer += txt;
    }
    // Update label text
    lv_label_set_text(input_label, input_buffer.c_str());
}

// Create keyboard (call once when building editor page, or rely on lazy init below)
static void create_keyboard_if_needed(lv_obj_t *parent) {
    if (keyboard && lv_obj_is_valid(keyboard)) return;
    // parent should be a screen or persistent container. Use lv_scr_act() to be safe across pages:
    lv_obj_t *kbd_parent = parent ? parent : lv_scr_act();
    keyboard = lv_keyboard_create(kbd_parent);
    lv_obj_set_size(keyboard, 480, 120);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    // Set custom keymap
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1, custom_kb_map, custom_kb_ctrl);
    // Force keyboard to use this custom mode
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);
    // Set custom grid to make keys taller and less wide

    // Widths: 3 equal columns, Height: 5 equal rows
    static lv_coord_t col_dsc[] = {80, 80, 80, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {40, 40, 40, 40, 40, LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(keyboard, col_dsc, row_dsc);

    // Tell LVGL to align buttons into this grid
    lv_obj_set_style_pad_all(keyboard, 2, 0); // padding between buttons
    lv_obj_set_size(keyboard, 240, 200);      // set overall size
    lv_obj_set_pos(keyboard, -120, 0);       // position below textarea

    lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}



static void ta_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e); // the textarea
    if(code == LV_EVENT_FOCUSED) {
        // Ensure keyboard exists and is valid
        if(!keyboard || !lv_obj_is_valid(keyboard)) {
            // try to create it on-demand using the current screen as parent
            create_keyboard_if_needed(lv_scr_act());
            if(!keyboard || !lv_obj_is_valid(keyboard)) {
                // Creation failed — safe fallback: do nothing
                printf("Warning: keyboard not available\n");
                return;
            }
        }
        // Attach the textarea to keyboard and show it
        lv_keyboard_set_textarea(keyboard, ta);
        lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        // If numeric-only fields, set keyboard mode here as desired:
        // lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_NUMBER);
    }
    else if(code == LV_EVENT_DEFOCUSED) {
        // Detach and hide keyboard, but only if keyboard is valid
        if(keyboard && lv_obj_is_valid(keyboard)) {
            lv_keyboard_set_textarea(keyboard, NULL);
            lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        }
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
        lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_DEFOCUSED, NULL);
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


    create_keyboard_if_needed(page_editor); // ensures keyboard exists and is parented safely
    // Label above keyboard
    input_label = lv_label_create(page_editor);
    lv_obj_set_width(input_label, 480);
    lv_label_set_text(input_label, "");
    lv_obj_set_style_text_align(input_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(input_label, 5, 5);
}

void Setup_lvgl_selector() {
    build_home_page();
    build_editor_page();
    build_diag_page();
    // Start on auton page
    show_page(page_home);
}
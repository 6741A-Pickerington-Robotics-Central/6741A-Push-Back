#include "robot/ui/ui_main.hpp"

std::string selected_auton = "";
const int TEMP_THRESHOLD = 50; // Temperature threshold for highlighting
lv_obj_t *btn_diag;
bool any_motor_over_temp = false;

void load_auton_event();

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

void runtxtauton(std::vector<std::string> list) {
    chassis.setPose(0, 0, 0); // Reset the chassis pose to (0, 0, 0) before running the auton
    if (list.empty()) {
      printf("The list is empty. No action taken.\n");
      return;
    }
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
#endif

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
static void goto_editor(lv_event_t *e) { show_page(page_editor); load_auton_event(); }
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

// Track selection
static int selected_index = 0;
static int visible_offset = 0;
static const int VISIBLE_COUNT = 3; // number of visible containers
static lv_obj_t* list_inner;
static char number_input[16];        // current typed number (as string)
static lv_obj_t* active_button = NULL; // button that was clicked to open popup
lv_obj_t* display_label;
void save_auton_event(lv_event_t* e);

//Colors
lv_color_t purple = lv_palette_main(LV_PALETTE_PURPLE);
lv_color_t red    = lv_palette_main(LV_PALETTE_RED);
lv_color_t green  = lv_palette_main(LV_PALETTE_GREEN);
lv_color_t blue   = lv_palette_main(LV_PALETTE_BLUE);
lv_color_t yellow = lv_palette_main(LV_PALETTE_YELLOW);

// Open a popup to enter a number
void number_key_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    const char* digit = lv_label_get_text(lv_obj_get_child(btn, 0));
    int len = strlen(number_input);
    if (len < 15) {
        number_input[len] = digit[0];
        number_input[len+1] = '\0';
    }
    // update popup display
    lv_obj_t* popup_label = (lv_obj_t*)lv_event_get_user_data(e);
    lv_label_set_text(popup_label, number_input);
}

void dot_key_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    const char* digit = ".";
    int len = strlen(number_input);
    if (len < 15) {
        number_input[len] = digit[0];
        number_input[len+1] = '\0';
    }
    // update popup display
    lv_obj_t* popup_label = (lv_obj_t*)lv_event_get_user_data(e);
    lv_label_set_text(popup_label, number_input);
}

void backspace_event(lv_event_t* e) {
    int len = strlen(number_input);
    if (len > 0) number_input[len-1] = '\0';
    lv_obj_t* popup_label = (lv_obj_t*)lv_event_get_user_data(e);
    lv_label_set_text(popup_label, number_input);
}

void enter_event(lv_event_t* e) {
    if (active_button) {
        if (number_input[0] == '\0') {
            // number_input is blank → show "0"
            strcpy(number_input, "0");
            lv_label_set_text(lv_obj_get_child(active_button, 0), "0");
        } else {
            // number_input has content → show that instead
            lv_label_set_text(lv_obj_get_child(active_button, 0), number_input);
        }
    }
    // Delete the popup container (parent of the Enter button)
    lv_obj_t* popup = lv_obj_get_parent(lv_event_get_current_target(e));
    lv_obj_del(popup);
    active_button = NULL;
}

void open_number_key_popup(lv_event_t* e) {
    active_button = lv_event_get_target(e);
    const char* current = lv_label_get_text(lv_obj_get_child(active_button, 0));
    strncpy(number_input, current, sizeof(number_input));
    number_input[sizeof(number_input)-1] = '\0';

    lv_obj_t* parent = lv_scr_act();
    lv_obj_t* popup = lv_obj_create(parent);
    lv_obj_set_size(popup, 300, 210);
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(popup, LV_ALIGN_CENTER, -20, 0);
    lv_obj_set_style_bg_color(popup, purple, 0);
    lv_obj_set_style_border_width(popup, 0, 0);
    lv_obj_set_style_border_color(popup, purple, 0);

    // Label to show current input
    lv_obj_t* display_label = lv_label_create(popup);
    if (strcmp(number_input, "0") == 0)
    {
        number_input[0] = '\0';
        lv_label_set_text(display_label, "");
    } else {
        lv_label_set_text(display_label, number_input);
    }
    
    lv_obj_set_pos(display_label, 10, 0);

    // Manually create buttons 0-9
    int btn_size = 50;
    int x0 = 0, y0 = 20;
    for (int i = 1; i <= 9; i++) {
        lv_obj_t* btn = lv_btn_create(popup);
        lv_obj_set_size(btn, btn_size, btn_size);
        int row = (i-1)/3;
        int col = (i-1)%3;
        lv_obj_set_pos(btn, x0 + col*(btn_size+5), y0 + row*(btn_size+5));

        lv_obj_t* lbl = lv_label_create(btn);
        char buf[2]; buf[0] = '0' + i; buf[1] = '\0';
        lv_label_set_text(lbl, buf);
        lv_obj_center(lbl);

        lv_obj_add_event_cb(btn, number_key_event, LV_EVENT_CLICKED, display_label);
    }

    // Button 0
    lv_obj_t* btn0 = lv_btn_create(popup);
    lv_obj_set_size(btn0, btn_size, btn_size);
    lv_obj_set_pos(btn0, x0 + 3*(btn_size+5), y0 + 0*(btn_size+5));
    lv_obj_t* lbl0 = lv_label_create(btn0);
    lv_label_set_text(lbl0, "0");
    lv_obj_center(lbl0);
    lv_obj_add_event_cb(btn0, number_key_event, LV_EVENT_CLICKED, display_label);

    // Button .
    lv_obj_t* btndot = lv_btn_create(popup);
    lv_obj_set_size(btndot, btn_size, btn_size);
    lv_obj_set_pos(btndot, x0 + 3*(btn_size+5), y0 + 1*(btn_size+5));
    lv_obj_t* lbldot = lv_label_create(btndot);
    lv_label_set_text(lbldot, ".");
    lv_obj_center(lbldot);
    lv_obj_add_event_cb(btndot, dot_key_event, LV_EVENT_CLICKED, display_label);

    // Backspace button
    lv_obj_t* btn_back = lv_btn_create(popup);
    lv_obj_set_size(btn_back, 50, btn_size);
    lv_obj_set_pos(btn_back, x0 + 3*(btn_size+5), y0 + 2*(btn_size+5));
    lv_obj_t* lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, "<-");
    lv_obj_center(lbl_back);
    lv_obj_add_event_cb(btn_back, backspace_event, LV_EVENT_CLICKED, display_label);

    // Enter button
    lv_obj_t* btn_enter = lv_btn_create(popup);
    lv_obj_set_size(btn_enter, 50, (btn_size+5)*3 - 5);
    lv_obj_set_pos(btn_enter, x0 + 4*(btn_size+5), y0 + 0*(btn_size+5));
    lv_obj_t* lbl_enter = lv_label_create(btn_enter);
    lv_label_set_text(lbl_enter, "Enter");
    lv_obj_center(lbl_enter);
    lv_obj_add_event_cb(btn_enter, enter_event, LV_EVENT_CLICKED, NULL);
}

void open_number_speed_popup(lv_event_t* e) {
    active_button = lv_event_get_target(e);
    const char* current = lv_label_get_text(lv_obj_get_child(active_button, 0));
    strncpy(number_input, current, sizeof(number_input));
    number_input[sizeof(number_input)-1] = '\0';

    lv_obj_t* parent = lv_scr_act();
    lv_obj_t* popup = lv_obj_create(parent);
    lv_obj_set_size(popup, 200, 180);
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(popup, LV_ALIGN_CENTER, -20, 0);
    lv_obj_set_style_bg_color(popup, purple, 0);
    lv_obj_set_style_border_width(popup, 0, 0);
    lv_obj_set_style_border_color(popup, purple, 0);

    // Label to show current input
    display_label = lv_label_create(popup);
    lv_label_set_text(display_label, number_input);
    lv_obj_align(display_label, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t* arc = lv_arc_create(popup);
    lv_obj_set_size(arc, 120, 120);
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, -20);
    lv_arc_set_range(arc, 0, 270);
    lv_arc_set_value(arc, 0);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);

    // Update label and number_input when arc changes
    lv_obj_add_event_cb(arc, [](lv_event_t* e) {
        lv_obj_t* arc = lv_event_get_target(e);
        int value = lv_arc_get_value(arc);
        snprintf(number_input, sizeof(number_input), "%d", value);
        lv_label_set_text(display_label, number_input);
        lv_label_set_text(display_label, number_input);
    }, LV_EVENT_VALUE_CHANGED, display_label);

    // Optionally, set initial value from number_input if it's a valid integer
    int initial_value = atoi(number_input);
    lv_arc_set_value(arc, initial_value);
    lv_label_set_text(display_label, number_input);

    // Enter button
    lv_obj_t* btn_enter = lv_btn_create(popup);
    lv_obj_set_size(btn_enter, 150, 40);
    lv_obj_align(btn_enter, LV_ALIGN_CENTER, 0, 60);
    lv_obj_t* lbl_enter = lv_label_create(btn_enter);
    lv_label_set_text(lbl_enter, "Enter");
    lv_obj_center(lbl_enter);
    lv_obj_add_event_cb(btn_enter, enter_event, LV_EVENT_CLICKED, NULL);
}

void open_number_dir_popup(lv_event_t* e) {
    active_button = lv_event_get_target(e);
    const char* current = lv_label_get_text(lv_obj_get_child(active_button, 0));
    strncpy(number_input, current, sizeof(number_input));
    number_input[sizeof(number_input)-1] = '\0';

    lv_obj_t* parent = lv_scr_act();
    lv_obj_t* popup = lv_obj_create(parent);
    lv_obj_set_size(popup, 200, 190);
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(popup, LV_ALIGN_CENTER, -20, 0);
    lv_obj_set_style_bg_color(popup, purple, 0);
    lv_obj_set_style_border_width(popup, 0, 0);
    lv_obj_set_style_border_color(popup, purple, 0);

    // Label to show current input
    display_label = lv_label_create(popup);
    lv_label_set_text(display_label, number_input);
    lv_obj_align(display_label, LV_ALIGN_CENTER, 0, -25);

    lv_obj_t* arc = lv_arc_create(popup);
    lv_obj_set_size(arc, 120, 120);
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, -25);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    
    // Make the arc span 360 degrees and hide the indicator
    lv_arc_set_range(arc, -180, 180);
    lv_arc_set_rotation(arc, 90); // Start from top (12 o'clock)
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_INDICATOR);

    // Update label and number_input when arc changes
    lv_obj_add_event_cb(arc, [](lv_event_t* e) {
        lv_obj_t* arc = lv_event_get_target(e);
        int value = lv_arc_get_value(arc);
        snprintf(number_input, sizeof(number_input), "%d", value);
        lv_label_set_text(display_label, number_input);
        lv_label_set_text(display_label, number_input);
    }, LV_EVENT_VALUE_CHANGED, display_label);

    // Optionally, set initial value from number_input if it's a valid integer
    int initial_value = atoi(number_input);
    lv_arc_set_value(arc, initial_value);
    lv_label_set_text(display_label, number_input);

    // Enter button
    lv_obj_t* btn_enter = lv_btn_create(popup);
    lv_obj_set_size(btn_enter, 150, 40);
    lv_obj_align(btn_enter, LV_ALIGN_CENTER, 0, 65);
    lv_obj_t* lbl_enter = lv_label_create(btn_enter);
    lv_label_set_text(lbl_enter, "Enter");
    lv_obj_center(lbl_enter);
    lv_obj_add_event_cb(btn_enter, enter_event, LV_EVENT_CLICKED, NULL);
}

void open_options_popup(lv_event_t* e) {
    lv_obj_t* parent = lv_scr_act();
    lv_obj_t* popup = lv_obj_create(parent);
    lv_obj_set_size(popup, 160, 190);
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(popup, LV_ALIGN_CENTER, 80, 0);
    lv_obj_set_style_bg_color(popup, purple, 0);
    lv_obj_set_style_border_width(popup, 0, 0);
    lv_obj_set_style_border_color(popup, purple, 0);

    // Save button
    lv_obj_t* btn_save = lv_btn_create(popup);
    lv_obj_set_size(btn_save, 150, 40);
    lv_obj_align(btn_save, LV_ALIGN_CENTER, 0, -65);
    lv_obj_t* lbl_save = lv_label_create(btn_save);
    lv_label_set_text(lbl_save, "Save");
    lv_obj_center(lbl_save);
    lv_obj_add_event_cb(btn_save, save_auton_event, LV_EVENT_CLICKED, NULL);
    //20 for above close, 25 is spaceing (5 gap)
    // Close button
    lv_obj_t* btn_close = lv_btn_create(popup);
    lv_obj_set_size(btn_close, 150, 40);
    lv_obj_align(btn_close, LV_ALIGN_CENTER, 0, 65);
    lv_obj_t* lbl_close = lv_label_create(btn_close);
    lv_label_set_text(lbl_close, "Close");
    lv_obj_center(lbl_close);
    lv_obj_add_event_cb(btn_close, enter_event, LV_EVENT_CLICKED, NULL);
}

lv_obj_t* create_number_button(lv_obj_t* parent, const char* text, int type) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 50, 30);
    if (type == 0)
    {
        lv_obj_add_event_cb(btn, open_number_key_popup, LV_EVENT_CLICKED, NULL);
    } else if (type == 1)
    {
        lv_obj_add_event_cb(btn, open_number_speed_popup, LV_EVENT_CLICKED, NULL);
    } else if (type == 2)
    {
        lv_obj_add_event_cb(btn, open_number_dir_popup, LV_EVENT_CLICKED, NULL);
    }
    
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);

    return btn;
}

/*

// Bars for each command type
lv_obj_t* create_command_bar(lv_obj_t* parent, const char* type) {
    lv_obj_t* bar = lv_obj_create(parent);
    lv_obj_set_size(bar, 350, 55);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_cross_place(bar, LV_FLEX_ALIGN_CENTER, 0); // Center items vertically
    lv_obj_set_style_pad_all(bar, 4, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    if (strcmp(type, "move") == 0) {
        lv_obj_set_style_bg_color(bar, yellow, 0);
        lv_label_set_text(lv_label_create(bar), "Move to x,y:");
        create_number_button(bar, "0",0);
        create_number_button(bar, "0",0);
        lv_label_set_text(lv_label_create(bar), "dir:");
        create_number_button(bar, "0",2);
    } else if (strcmp(type, "wait") == 0) {
        lv_obj_set_style_bg_color(bar, green, 0);
        lv_label_set_text(lv_label_create(bar), "Wait");
        create_number_button(bar, "0",0);
        lv_label_set_text(lv_label_create(bar), "sec");
    } else if (strcmp(type, "motor") == 0) {
        lv_obj_set_style_bg_color(bar, blue, 0);
        lv_label_set_text(lv_label_create(bar), "Spin");
        lv_obj_t* motor = lv_dropdown_create(bar);
        lv_dropdown_set_options(motor, "Intake Top\nIntake Middle\nIntake Bottom");
        lv_label_set_text(lv_label_create(bar), "at");
        create_number_button(bar, "0",1);
    } else if (strcmp(type, "piston") == 0) {
        lv_obj_set_style_bg_color(bar, red, 0);
        lv_label_set_text(lv_label_create(bar), "Toggle");
        lv_obj_t* piston = lv_dropdown_create(bar);
        lv_dropdown_set_options(piston, "Descore\nWeedwacker");
        lv_label_set_text(lv_label_create(bar), "to");
        lv_obj_t* sw = lv_switch_create(bar);
    }
    return bar;
}

*/

// List to store bars
void update_list_position() {
    lv_obj_set_y(list_inner, -visible_offset * 55); // Move the inner container to simulate scrolling
    printf("Visible offset: %d\n", visible_offset);
    printf ("Inner list has %d children\n", (int)lv_obj_get_child_cnt(list_inner));
}

void highlight_selected() {
    uint32_t child_count = lv_obj_get_child_cnt(list_inner);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t* child = lv_obj_get_child(list_inner, i);

        if ((int)i == selected_index) {
            // Selected: white border, thick
            lv_obj_set_style_border_width(child, 4, 0);      // thick border
            lv_obj_set_style_border_color(child, lv_color_white(), 0);
            lv_obj_set_style_border_opa(child, LV_OPA_COVER, 0);
        } else {
            // Unselected: no border
            lv_obj_set_style_border_width(child, 0, 0);
        }
    }
}

void move_selection(int direction) {
    int child_count = lv_obj_get_child_cnt(list_inner);
    selected_index += direction;
    if (selected_index < 0) selected_index = 0;
    if (selected_index >= child_count) selected_index = child_count - 1;

    // If near edges, adjust visible offset
    if (selected_index < visible_offset) {
        visible_offset = selected_index;
    } else if (selected_index >= visible_offset + VISIBLE_COUNT) {
        visible_offset = selected_index - VISIBLE_COUNT + 1;
    }

    update_list_position();
    highlight_selected();
}

void up_event(lv_event_t* e) { move_selection(-1); }
void down_event(lv_event_t* e) { move_selection(1); }

// Page builder
void build_editor_page() {
    page_editor = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page_editor, 480, 240);
    lv_obj_clear_flag(page_editor, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(page_editor, 0, 0);
    lv_obj_set_style_pad_all(page_editor, 0, 0);
    lv_obj_set_style_bg_opa(page_editor, LV_OPA_TRANSP, 0);

    // ===== LIST MASK (acts as viewport) =====
    lv_obj_t* list_view = lv_obj_create(page_editor);
    lv_obj_set_size(list_view, 395, 240);
    lv_obj_set_pos(list_view, 0, 0);
    lv_obj_clear_flag(list_view, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(list_view, 0, 0);
    lv_obj_set_style_clip_corner(list_view, true, 0); // mask children outside bounds
    lv_obj_set_scrollbar_mode(list_view, LV_SCROLLBAR_MODE_OFF);

    // ===== INNER LIST =====
    list_inner = lv_obj_create(list_view);
    lv_obj_set_size(list_inner, 395, LV_SIZE_CONTENT); // very tall to allow many items
    lv_obj_set_flex_flow(list_inner, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list_inner, 4, 0); // spacing between bars
    lv_obj_clear_flag(list_inner, LV_OBJ_FLAG_SCROLLABLE);

    highlight_selected();

    // Add button
    lv_obj_t *btn_add = lv_btn_create(page_editor);
    lv_obj_set_size(btn_add, 70, 40);
    lv_obj_set_pos(btn_add, 405, 5);
    //lv_obj_add_event_cb(btn_add, add_command_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_add = lv_label_create(btn_add);
    lv_label_set_text(lbl_add, "Add");
    lv_obj_center(lbl_add);

    // Remove button
    lv_obj_t *btn_remove = lv_btn_create(page_editor);
    lv_obj_set_size(btn_remove, 70, 40);
    lv_obj_set_pos(btn_remove, 405, 50);
    //lv_obj_add_event_cb(btn_remove, add_command_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_remove = lv_label_create(btn_remove);
    lv_label_set_text(lbl_remove, "Del");
    lv_obj_center(lbl_remove);

    // Up button
    lv_obj_t *btn_up = lv_btn_create(page_editor);
    lv_obj_set_size(btn_up, 70, 40);
    lv_obj_set_pos(btn_up, 405, 95);
    lv_obj_add_event_cb(btn_up, up_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_up = lv_label_create(btn_up);
    lv_label_set_text(lbl_up, "UP");
    lv_obj_center(lbl_up);

    // Down button
    lv_obj_t *btn_down = lv_btn_create(page_editor);
    lv_obj_set_size(btn_down, 70, 40);
    lv_obj_set_pos(btn_down, 405, 140);
    lv_obj_add_event_cb(btn_down, down_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_down = lv_label_create(btn_down);
    lv_label_set_text(lbl_down, "Down");
    lv_obj_center(lbl_down);
    
    // Options button
    lv_obj_t *btn_options = lv_btn_create(page_editor);
    lv_obj_set_size(btn_options, 70, 40);
    lv_obj_set_pos(btn_options, 405, 185);
    lv_obj_add_event_cb(btn_options, open_options_popup, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_options = lv_label_create(btn_options);
    lv_label_set_text(lbl_options, "OPT");
    lv_obj_center(lbl_down);
}

void load_auton_event() {
    std::ifstream file("/usd/auton.txt");
    if (!file.is_open()) return;

    lv_obj_clean(list_inner);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::string type;
        switch (line[0]) {
            case 'm': type = "move"; break;
            case 'w': type = "wait"; break;
            case 's': type = "motor"; break;
            case 'p': type = "piston"; break;
            default: continue;
        }

        CommandBar cmd = CommandBar::create(list_inner, type);
        cmd.deserialize(line);
    }
    file.close();
}

void save_auton_event(lv_event_t* e) {
    std::ofstream file("/usd/auton.txt");
    if (!file.is_open()) return;

    uint32_t count = lv_obj_get_child_cnt(list_inner);
    for (uint32_t i = 0; i < count; i++) {
        lv_obj_t* bar = lv_obj_get_child(list_inner, i);
        lv_color_t bg = lv_obj_get_style_bg_color(bar, 0);
        std::string type;
        if (lv_color_to32(bg) == lv_color_to32(lv_palette_main(LV_PALETTE_YELLOW))) type = "move";
        else if (lv_color_to32(bg) == lv_color_to32(lv_palette_main(LV_PALETTE_GREEN))) type = "wait";
        else if (lv_color_to32(bg) == lv_color_to32(lv_palette_main(LV_PALETTE_BLUE))) type = "motor";
        else if (lv_color_to32(bg) == lv_color_to32(lv_palette_main(LV_PALETTE_RED))) type = "piston";

        CommandBar cmd{bar, type};
        file << cmd.serialize() << "\n";
    }
    file.close();
}

/*

void save_auton_event(lv_event_t* e) {
    // Open file on SD card (adjust path if you want to use `/usd/auton.txt`)
    std::ofstream file("/usd/auton.txt");
    if (!file.is_open()) {
        printf("Failed to open file!\n");
        return;
    }
    uint32_t count = lv_obj_get_child_cnt(list_inner);
    for (uint32_t i = 0; i < count; i++) {
        lv_obj_t* bar = lv_obj_get_child(list_inner, i);
        lv_color_t bg = lv_obj_get_style_bg_color(bar, 0);
        // We'll detect type by color (or you can add a hidden label to store "type")
        std::string type;
        char cmd_letter = '?';
        if (lv_color_to32(bg) == lv_color_to32(lv_palette_main(LV_PALETTE_YELLOW))) {
            type = "move";
            cmd_letter = 'm';
        } else if (lv_color_to32(bg) == lv_color_to32(lv_palette_main(LV_PALETTE_GREEN))) {
            type = "wait";
            cmd_letter = 'w';
        } else if (lv_color_to32(bg) == lv_color_to32(lv_palette_main(LV_PALETTE_BLUE))) {
            type = "motor";
            cmd_letter = 's';
        } else if (lv_color_to32(bg) == lv_color_to32(lv_palette_main(LV_PALETTE_RED))) {
            type = "piston";
            cmd_letter = 'p';
        }
        std::string line;
        // === Move Command ===
        if (type == "move") {
            // Child structure: [“Move to x,y:”][btnX][btnY][“dir:”][btnDir]
            lv_obj_t* btnX = lv_obj_get_child(bar, 1);
            lv_obj_t* btnY = lv_obj_get_child(bar, 2);
            lv_obj_t* btnDir = lv_obj_get_child(bar, 4);
            const char* x = lv_label_get_text(lv_obj_get_child(btnX, 0));
            const char* y = lv_label_get_text(lv_obj_get_child(btnY, 0));
            const char* dir = lv_label_get_text(lv_obj_get_child(btnDir, 0));
            line = "m,r," + std::string(x) + "," + std::string(y) + "," + std::string(dir);
        }
        // === Wait Command ===
        else if (type == "wait") {
            // Structure: [“Wait”][btn][“sec”]
            lv_obj_t* btn = lv_obj_get_child(bar, 1);
            const char* sec = lv_label_get_text(lv_obj_get_child(btn, 0));
            line = "w,r," + std::string(sec);
        }
        // === Motor Command ===
        else if (type == "motor") {
            // [“Spin”][dropdown][“at”][btn]
            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* btn = lv_obj_get_child(bar, 3);
            const char* speed = lv_label_get_text(lv_obj_get_child(btn, 0));
            // Find dropdown selected option
            char buffer[32];
            lv_dropdown_get_selected_str(dropdown, buffer, sizeof(buffer));
            char motor_letter = 'a'; // default
            if (strcmp(buffer, "Intake Top") == 0) motor_letter = 'a';
            else if (strcmp(buffer, "Intake Middle") == 0) motor_letter = 'b';
            else if (strcmp(buffer, "Intake Bottom") == 0) motor_letter = 'c';
            line = "s," + std::string(1, motor_letter) + "," + std::string(speed);
        }
        // === Piston Command ===
        else if (type == "piston") {
            // [“Toggle”][dropdown][“to”][switch]
            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* sw = lv_obj_get_child(bar, 3);
            char buffer[32];
            lv_dropdown_get_selected_str(dropdown, buffer, sizeof(buffer));
            char piston_letter = 'd'; // default
            if (strcmp(buffer, "Descore") == 0) piston_letter = 'd';
            else if (strcmp(buffer, "Weedwacker") == 0) piston_letter = 'w';
            int state = lv_obj_has_state(sw, LV_STATE_CHECKED) ? 1 : 0;
            line = "p," + std::string(1, piston_letter) + "," + std::to_string(state);
        }
        file << line << "\n"; // Write to file
    }
    file.close();
    printf("Auton saved!\n");
}

void load_auton_event() {
    std::ifstream file("/usd/auton.txt");
    if (!file.is_open()) {
        printf("Failed to open file!\n");
        return;
    }
    // Clear existing bars
    lv_obj_clean(list_inner); // Clear existing bars first
    lv_obj_clear_flag(list_inner, LV_OBJ_FLAG_HIDDEN);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            printf("Empty line in auton file, skipping.\n"); 
            continue;
        }
        printf("Read line: %s\n", line.c_str());
        char cmd = line[0];
        lv_obj_t* bar = NULL;
        if (cmd == 'm') {
            // Move command: m,r,x,y,dir
            bar = create_command_bar(list_inner, "move");
            size_t pos1 = line.find(',', 4);
            size_t pos2 = line.find(',', pos1 + 1);
            size_t pos3 = line.find(',', pos2 + 1);

            std::string x = line.substr(4, pos1 - 4);
            std::string y = line.substr(pos1 + 1, pos2 - pos1 - 1);
            std::string dir = line.substr(pos2 + 1);

            lv_obj_t* btnX = lv_obj_get_child(bar, 1);
            lv_obj_t* btnY = lv_obj_get_child(bar, 2);
            lv_obj_t* btnDir = lv_obj_get_child(bar, 4);

            lv_label_set_text(lv_obj_get_child(btnX, 0), x.c_str());
            lv_label_set_text(lv_obj_get_child(btnY, 0), y.c_str());
            lv_label_set_text(lv_obj_get_child(btnDir, 0), dir.c_str());

            printf("Created move bar\n");
        } 
        else if (cmd == 'w') {
            // Wait command: w,r,seconds
            bar = create_command_bar(list_inner, "wait");
            std::string sec = line.substr(4);
            lv_obj_t* btn = lv_obj_get_child(bar, 1);
            lv_label_set_text(lv_obj_get_child(btn, 0), sec.c_str());
        } 
        else if (cmd == 's') {
            // Motor command: s,motor_letter,speed
            bar = create_command_bar(list_inner, "motor");
            char motor_letter = line[2];
            std::string speed = line.substr(4);

            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* btn = lv_obj_get_child(bar, 3);

            if (motor_letter == 'a') lv_dropdown_set_selected(dropdown, 0);
            else if (motor_letter == 'b') lv_dropdown_set_selected(dropdown, 1);
            else if (motor_letter == 'c') lv_dropdown_set_selected(dropdown, 2);

            lv_label_set_text(lv_obj_get_child(btn, 0), speed.c_str());
        } 
        else if (cmd == 'p') {
            // Piston command: p,piston_letter,state
            bar = create_command_bar(list_inner, "piston");
            char piston_letter = line[2];
            std::string state_str = line.substr(4);

            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* sw = lv_obj_get_child(bar, 3);

            if (piston_letter == 'd') lv_dropdown_set_selected(dropdown, 0);
            else if (piston_letter == 'w') lv_dropdown_set_selected(dropdown, 1);

            if (state_str == "1") lv_obj_add_state(sw, LV_STATE_CHECKED);
            else lv_obj_clear_state(sw, LV_STATE_CHECKED);
        }
        else {
            printf("Unknown command type: %c\n", cmd);
            continue; // skip unknown commands
        }
        lv_obj_update_layout(list_inner); // Update layout after adding
        printf("Bar %p: x=%d y=%d w=%d h=%d hidden=%d children=%d\n", bar,
       (int)lv_obj_get_x(bar),
       (int)lv_obj_get_y(bar),
       (int)lv_obj_get_width(bar),
       (int)lv_obj_get_height(bar),
       lv_obj_has_flag(bar, LV_OBJ_FLAG_HIDDEN),
       (int)lv_obj_get_child_cnt(bar));
    }
    file.close();
    // After loading, make sure to refresh selection / list
    selected_index = 0;
    visible_offset = 0;
    update_list_position();
    highlight_selected();
    lv_obj_update_layout(list_inner); // Update layout after adding
    printf("Parent: %p, hidden=%d, opa=%d, w=%d, h=%d\n",
       list_inner,
       lv_obj_has_flag(list_inner, LV_OBJ_FLAG_HIDDEN),
       lv_obj_get_style_bg_opa(list_inner, 0),
       lv_obj_get_width(list_inner),
       lv_obj_get_height(list_inner));

}

*/

void Setup_lvgl_selector() {
    build_home_page();
    build_editor_page();
    build_diag_page();
    // Start on auton page
    show_page(page_home);
}
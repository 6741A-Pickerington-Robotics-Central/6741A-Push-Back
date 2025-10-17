#include "robot/ui/commandbar.hpp"
#include "robot/ui/popup.hpp"
#include "robot/robot.hpp"
#include "robot/ezlog.hpp"
// General headers
#include "pros/motors.h"
#include "pros/rtos.hpp"
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "liblvgl/lvgl.h"
#include <sys/stat.h>
#include <sys/types.h>

void load_auton_event(std::string filename);
// Global vars
std::string selected_auton = "";
const int TEMP_THRESHOLD = 50;
lv_obj_t *btn_diag;
bool any_motor_over_temp = false;
int selected_index = 0;
int visible_offset = 0;
const int VISIBLE_COUNT = 3;
lv_obj_t* list_inner = nullptr;
char number_input[16] = "";
lv_obj_t* active_button = nullptr;
lv_obj_t* display_label = nullptr;
int selected_corner = -1;  // 0=Red Left, 1=Red Right, 2=Blue Left, 3=Blue Right
int selected_slot = -1;  // 0,1,2

// Simple device list (you can expand this)
struct Device {
    std::string name;
    int port;
    bool is_motor;
    bool is_drive;
};

std::vector<Device> robotMotors = {
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

// --- Utility: Show one page, hide others ---
void show_page(lv_obj_t *page) {
    lv_obj_add_flag(page_home, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_editor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_diag, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(page, LV_OBJ_FLAG_HIDDEN);
}

std::string get_auton_file_path() {
    if(selected_corner < 0 || selected_corner > 3 || selected_slot < 0 || selected_slot > 2)
        return ""; // invalid selection

    const char* corner_names[] = { "red_left", "red_right", "blue_left", "blue_right" };
    char buf[128];
    snprintf(buf, sizeof(buf), "autons/%s/slot%d.txt",
             corner_names[selected_corner],
             selected_slot + 1);
    return std::string(buf);
}

// --- Event callbacks for switch pages ---
void goto_home(lv_event_t *e) { show_page(page_home); }
void goto_editor(lv_event_t *e) { 
    if (selected_corner == -1 || selected_slot == -1) return; // nothing selected
    show_page(page_editor); 
    load_auton_event(get_auton_file_path()); 
    int selected_index = 0;
    int visible_offset = 0;
    highlight_selected();
}
void goto_diag(lv_event_t *e) { show_page(page_diag); }


///////////////////////
// --- Home Page --- //
///////////////////////


lv_obj_t* auton_list; // global
lv_obj_t* auton_label; // global
lv_obj_t* main_area; // global
void corner_btn_cb(lv_event_t* e);

// Buttons for corners (to identify in callback)
lv_obj_t* red_left_btn;
lv_obj_t* red_right_btn;
lv_obj_t* blue_left_btn;
lv_obj_t* blue_right_btn;

// Slot button callback
void slot_btn_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    // Determine slot number
    const char* txt = lv_label_get_text(lv_obj_get_child(btn,0));
    sscanf(txt, "Slot %d", &selected_slot);
    selected_slot -= 1;
    // Corner names and colors
    const char* corner_names[] = { "Red Left", "Red Right", "Blue Left", "Blue Right" };
    const char* corner_colors[] = { "#FF0000", "#FF0000", "#0000FF", "#0000FF" };
    lv_label_set_recolor(auton_label, true);   
    char buf[128];
    if(selected_corner >= 0 && selected_corner < 4 && selected_slot >= 0) {
        // LVGL recolor uses #RRGGBBtext# syntax
        sprintf(buf, "Selected: %s %s# Slot %d",
                corner_colors[selected_corner],
                corner_names[selected_corner],
                selected_slot + 1);
    } else {
        sprintf(buf, "Selected: None");
    }
    lv_label_set_text(auton_label, buf);
}

void build_corner_list() {
    auton_list = lv_list_create(main_area);
    lv_obj_set_size(auton_list, 220, LV_PCT(100));  // ~half width
    // Add corner buttons
    red_left_btn = lv_list_add_btn(auton_list, NULL, "Red Left");
    red_right_btn = lv_list_add_btn(auton_list, NULL, "Red Right");
    blue_left_btn = lv_list_add_btn(auton_list, NULL, "Blue Left");
    blue_right_btn = lv_list_add_btn(auton_list, NULL, "Blue Right");
    // Color the buttons
    lv_obj_set_style_text_color(lv_obj_get_child(red_left_btn, 0), lv_color_hsv_to_rgb(10,255,255), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(red_right_btn, 0), lv_color_hsv_to_rgb(10,255,255), 0); 
    lv_obj_set_style_text_color(lv_obj_get_child(blue_left_btn, 0), lv_color_hsv_to_rgb(200,255,255), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(blue_right_btn, 0), lv_color_hsv_to_rgb(200,255,255), 0);
    // Add event callbacks
    lv_obj_add_event_cb(red_left_btn, corner_btn_cb, LV_EVENT_CLICKED, auton_label);
    lv_obj_add_event_cb(red_right_btn, corner_btn_cb, LV_EVENT_CLICKED, auton_label);
    lv_obj_add_event_cb(blue_left_btn, corner_btn_cb, LV_EVENT_CLICKED, auton_label);
    lv_obj_add_event_cb(blue_right_btn, corner_btn_cb, LV_EVENT_CLICKED, auton_label);
}

void back_to_corners_cb(lv_event_t* e) {
    lv_obj_del(auton_list);
    auton_list = NULL;
    build_corner_list();  // recreate the 4 corner buttons in left half
    lv_label_set_text(auton_label, "Selected: None");
}

void corner_btn_cb(lv_event_t* e) { // Corner button callback
    lv_obj_t* btn = lv_event_get_target(e);
    // figure out corner index (could store in user_data or use pointer comparison)
    if (btn == red_left_btn) selected_corner = 0;
    else if (btn == red_right_btn) selected_corner = 1;
    else if (btn == blue_left_btn) selected_corner = 2;
    else if (btn == blue_right_btn) selected_corner = 3;
    // Update top label
    lv_label_set_text(auton_label, "Selected: None");
    // Remove previous slot list if exists
    // Remove the corner buttons list
    if (auton_list) {
        lv_obj_del(auton_list);
        auton_list = NULL;
    }
    // Create slot buttons
    auton_list = lv_list_create(main_area); // parent = main_area
    lv_obj_set_size(auton_list, 220, LV_PCT(100));

    for (int i = 0; i < 3; i++) {
        char buf[16];
        sprintf(buf, "Slot %d", i + 1);
        lv_obj_t* s_btn = lv_list_add_btn(auton_list, NULL, buf);
        lv_obj_add_event_cb(s_btn, slot_btn_cb, LV_EVENT_CLICKED, auton_label);
        lv_obj_set_style_text_color(lv_obj_get_child(s_btn,0),
                                    lv_color_hsv_to_rgb(100,255,255), 0);
    }
    lv_obj_t* back_btn = lv_list_add_btn(auton_list, NULL, "< Back");
    lv_obj_add_event_cb(back_btn, back_to_corners_cb, LV_EVENT_CLICKED, NULL);
}

void run_skills_task(void* param) {
    skills_auton();
}

// Free function for LVGL event
void skills_run_now_cb(lv_event_t* e) {
    // Delete the popup first
    lv_obj_del(lv_obj_get_parent(lv_event_get_current_target(e)));

    // Start the task (must pass all 4 arguments)
    pros::Task([] {
        pros::delay(1000); // slight delay to allow user to move away
        skills_auton();
    }, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "SkillsTask");

}

// Free function for Select button
void skills_select_cb(lv_event_t* e) {
    selected_corner = -2;
    selected_slot = 0;
    lv_label_set_recolor(auton_label, true);   
    char buf[128];
    sprintf(buf, "Selected: #00dd30ff Skills #");
    lv_label_set_text(auton_label, buf);

    // Delete popup
    lv_obj_del(lv_obj_get_parent(lv_event_get_current_target(e)));
}

// Event callback for the Skills button
static void skills_btn_event_cb(lv_event_t* e) {
    // Create a modal popup
    lv_obj_t *popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(popup, 180, 125);
    lv_obj_center(popup);
    lv_obj_set_style_bg_color(popup, lv_color_white(), 0);
    lv_obj_set_style_border_width(popup, 4, 0);
    lv_obj_set_style_border_color(popup, lv_color_black(), 0);
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE); // prevent scrolling inside popup

    // Run Now button
    lv_obj_t *btn_run_now = lv_btn_create(popup);
    lv_obj_set_size(btn_run_now, 160, 40);
    lv_obj_align(btn_run_now, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_t *lbl_run_now = lv_label_create(btn_run_now);
    lv_label_set_text(lbl_run_now, "Run Skills Now");
    lv_obj_center(lbl_run_now);

    // Select button
    lv_obj_t *btn_select = lv_btn_create(popup);
    lv_obj_set_size(btn_select, 160, 40);
    lv_obj_align(btn_select, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_t *lbl_select = lv_label_create(btn_select);
    lv_label_set_text(lbl_select, "Select Skills To Run");
    lv_obj_center(lbl_select);

    // Add event callbacks to the buttons
    lv_obj_add_event_cb(btn_run_now, skills_run_now_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(btn_select, skills_select_cb, LV_EVENT_CLICKED, NULL);
}

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

    auton_label = lv_label_create(top_bar);
    lv_label_set_text(auton_label, "Selected: None");
    lv_obj_set_style_text_color(auton_label, lv_color_black(), 0);
    lv_label_set_recolor(auton_label, true); // enable [color] tags
    lv_obj_center(auton_label);

    // Main area below top bar
    main_area = lv_obj_create(page_home);
    lv_obj_set_size(main_area, 480, 200);  // fills space under top bar
    lv_obj_set_pos(main_area, 0, 40);
    lv_obj_set_flex_flow(main_area, LV_FLEX_FLOW_ROW);  // horizontal split
    lv_obj_set_style_pad_all(main_area, 5, 0);
    lv_obj_set_style_bg_color(main_area, lv_color_hsv_to_rgb(280,255,255), 0);
    lv_obj_set_style_text_opa(main_area, LV_OPA_COVER, 0);

    // Left half with buttons
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
    lv_label_set_text(lbl_run, "Skills");
    lv_obj_set_pos(btn_run, 0, 120);
    lv_obj_set_size(btn_run, LV_PCT(100), 30);
    lv_obj_add_event_cb(btn_run, skills_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // Right half with auton list
    build_corner_list(); // populate with corner buttons
}


//////////////////////////////
// --- Diagnostics Page --- //
//////////////////////////////


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

    for (int i = 0; i < robotMotors.size(); i++) {
        if (!robotMotors[i].is_motor) continue;
        lv_obj_t *lbl;
        if(robotMotors[i].is_drive) {
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
        pros::Motor m(robotMotors[i].port);
        int temp = m.get_temperature();
        // Initial text
        if(robotMotors[i].is_drive) {
            lv_label_set_text_fmt(lbl, "%dC", temp);
        } else {
            lv_label_set_text_fmt(lbl, "%s - %dC", robotMotors[i].name.c_str(), temp);
        }
        // Timer data
        MotorLabelData *data = new MotorLabelData;
        data->label = lbl;
        data->port = robotMotors[i].port;
        data->threshold = TEMP_THRESHOLD;
        data->default_color = lv_color_white();
        if(!robotMotors[i].is_drive) data->name = robotMotors[i].name; // only set name for non-drive motors
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


/////////////////////////
// --- Editor Page --- //
/////////////////////////


int first_visible_index = 0; // Start index of current visible range
const int window_size = 3; // number of bars fully visible
const int buffer_above = 1; // one bar off-screen above
const int buffer_below = 2; // two bars off-screen below
const int bar_height = 55; // adjust if you change bar height
std::vector<CommandBar> visible_bars;
std::vector<CommandData> command_list;

static bool holding_up = false;
static bool holding_down = false;
static uint32_t press_start_time_up = 0;
static uint32_t press_start_time_down = 0;
static uint32_t last_repeat_time = 0;

constexpr uint32_t HOLD_THRESHOLD = 300; // ms to become a hold
constexpr uint32_t REPEAT_INTERVAL = 200; // ms between repeats

void ensure_selected_visible() {
    int first_visible = first_visible_index;
    int last_visible  = first_visible + window_size - 1; // use window_size
    if (selected_index < first_visible) {
        first_visible_index = selected_index;
    } else if (selected_index > last_visible) {
        first_visible_index = selected_index - window_size + 1;
    }
    render_visible_range(); // redraw visible bars
}

void update_command_list_from_bar(lv_obj_t* bar) {
    intptr_t idx = (intptr_t)lv_obj_get_user_data(bar);
    if (idx >= 0 && idx < (intptr_t)command_list.size()) {
        command_list[(size_t)idx].line = serialize_bar(bar);
        printf("Updated command_list[%ld] to %s\n", idx, command_list[(size_t)idx].line.c_str());
    }
}

lv_obj_t* create_number_button(lv_obj_t* parent, const char* text, int type) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 50, 30);
    if (type == 0)
    {
        lv_obj_add_event_cb(btn, PopupManager::openNumberKey, LV_EVENT_CLICKED, btn);
    } else if (type == 1)
    {
        lv_obj_add_event_cb(btn, PopupManager::openSpeed, LV_EVENT_CLICKED, btn);
    } else if (type == 2)
    {
        lv_obj_add_event_cb(btn, PopupManager::openDir, LV_EVENT_CLICKED, btn);
    }
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);

    return btn;
}

void render_visible_range() {
    visible_bars.clear();
    lv_obj_clean(list_inner);
    printf("fine1\n");
    // Determine start and end of the render range
    int start = std::max(first_visible_index - buffer_above, 0);
    int end = std::min(first_visible_index + window_size + buffer_below,(int)command_list.size());
    printf("fine2\n");
    for (int i = start; i < end; ++i) {
        printf("t1\n");
        CommandBar cmd = CommandBar::create(list_inner, command_list[i].type);
        printf("t2\n");
        cmd.deserialize(command_list[i].line);
        // Attach the global index 'i' to the bar so popups can find the model entry.
        // Note: cast through intptr_t to avoid warnings.
        printf("t3\n");
        lv_obj_set_user_data(cmd.bar, (void*)(intptr_t)i);
        printf("t4\n");
        visible_bars.push_back(cmd);
        printf("t5\n");
    }
    printf("fine3\n");
    // Shift the whole container upward so top buffer bar starts off-screen
    if (first_visible_index > 0) {
        lv_obj_set_y(list_inner, -bar_height * buffer_above);
    } else {
        lv_obj_set_y(list_inner, 0);
    }
    printf("fine4\n");
    highlight_selected();
    printf("not me2\n");
}

void animate_scroll(int direction) {
    static bool anim_running = false;
    if (anim_running) return;
    anim_running = true;

    int start_y = lv_obj_get_y(list_inner);
    int end_y = start_y - direction * bar_height;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, list_inner);
    lv_anim_set_values(&a, start_y, end_y);
    lv_anim_set_time(&a, 150); // scroll speed
    lv_anim_set_exec_cb(&a, [](void* obj, int32_t v) {
        lv_obj_set_y((lv_obj_t*)obj, v);
    });
    lv_anim_set_ready_cb(&a, [](lv_anim_t* a) {
        render_visible_range();
        anim_running = false;
    });
    lv_anim_start(&a);
}

void highlight_selected() {
    uint32_t count = lv_obj_get_child_cnt(list_inner);
    int start = std::max(first_visible_index - buffer_above, 0);

    for (uint32_t i = 0; i < count; i++) {
        lv_obj_t* child = lv_obj_get_child(list_inner, i);
        int global_index = start + i;

        if (global_index == selected_index) {
            lv_obj_set_style_border_width(child, 4, 0);
            lv_obj_set_style_border_color(child, lv_color_white(), 0);
            lv_obj_set_style_border_opa(child, LV_OPA_COVER, 0);
        } else {
            lv_obj_set_style_border_width(child, 4, 0);
            lv_obj_set_style_border_color(child, lv_color_black(), 0);
            lv_obj_set_style_border_opa(child, LV_OPA_COVER, 0);
        }
    }
}

void move_selection(int direction) {
    if (command_list.empty()) return;

    selected_index += direction;
    selected_index = std::clamp(selected_index, 0, (int)command_list.size() - 1);

    if (selected_index < first_visible_index) {
        first_visible_index = selected_index;
        animate_scroll(-1);
    } 
    else if (selected_index >= first_visible_index + window_size) {
        first_visible_index = selected_index - window_size + 1;
        animate_scroll(1);
    } 
    else {
        highlight_selected();
    }
}

void up_event(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    uint32_t now = pros::millis(); // or lv_tick_get()
    if (code == LV_EVENT_PRESSED) {
        press_start_time_up = now;
        holding_up = false; // reset
    }
    else if (code == LV_EVENT_PRESSING) {
        if (!holding_up && (now - press_start_time_up >= HOLD_THRESHOLD)) {
            holding_up = true; // now considered "held"
            last_repeat_time = now;
            move_selection(-1); // initial repeat
        } else if (holding_up && (now - last_repeat_time >= REPEAT_INTERVAL)) {
            move_selection(-1);
            last_repeat_time = now;
        }
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (!holding_up) {
            // short press -> regular tap
            move_selection(-1);
        }
        holding_up = false;
    }
}

void down_event(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    uint32_t now = pros::millis(); // or lv_tick_get()
    if (code == LV_EVENT_PRESSED) {
        press_start_time_down = now;
        holding_down = false;
    }
    else if (code == LV_EVENT_PRESSING) {
        if (!holding_down && (now - press_start_time_down >= HOLD_THRESHOLD)) {
            holding_down = true;
            last_repeat_time = now;
            move_selection(1); // initial repeat
        } else if (holding_down && (now - last_repeat_time >= REPEAT_INTERVAL)) {
            move_selection(1);
            last_repeat_time = now;
        }
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (!holding_down) {
            // short press -> regular tap
            move_selection(1);
        }
        holding_down = false;
    }
}

void delete_selected_command(lv_event_t* e) { // Deletes the currently selected command
    if (selected_index < 0 || selected_index >= (int)command_list.size()) return;

    // Remove from the data list
    command_list.erase(command_list.begin() + selected_index);
    // Adjust selected_index
    if (selected_index >= (int)command_list.size()) {
        selected_index = (int)command_list.size() - 1;
    }
    // Refresh the visible range
    ensure_selected_visible();
}

void load_auton_event(std::string filename) {
    std::string filepath = filename;
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    command_list.clear();

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        CommandData data;
        switch (line[0]) {
            case 'M': data.type = "movetopose"; break;
            case 'P': data.type = "movetopoint"; break; 
            case 't': data.type = "turn"; break;
            case 'w': data.type = "wait"; break;
            case 's': data.type = "motor"; break;
            case 'p': data.type = "piston"; break;
            default: continue;
        }
        data.line = line;
        command_list.push_back(data);
    }
    file.close();

    first_visible_index = 0;
    selected_index = 0;
    render_visible_range();
}

void save_auton_event(lv_event_t* e) {
    std::string filepath = get_auton_file_path();
    if(filepath.empty()) {
        printf("Error: invalid selection\n");
        return;
    }
    std::ofstream file(filepath);
    if(!file.is_open()) {
        printf("Error: could not open file %s\n", filepath.c_str());
        return;
    }
    for (const auto& cmd : command_list) {
        file << cmd.line << "\n";
    }
    file.close();
    printf("Auton saved to %s\n", filepath.c_str());
}

// Page builder
void build_editor_page() {
    page_editor = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page_editor, 480, 240);
    lv_obj_clear_flag(page_editor, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(page_editor, 0, 0);
    lv_obj_set_style_pad_all(page_editor, 0, 0);
    lv_obj_set_style_bg_opa(page_editor, LV_OPA_TRANSP, 0);

    // ===== INNER LIST =====
    list_inner = lv_obj_create(page_editor);
    lv_obj_set_size(list_inner, 395, LV_SIZE_CONTENT); // very tall to allow many items
    lv_obj_set_flex_flow(list_inner, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list_inner, 4, 0); // spacing between bars
    lv_obj_clear_flag(list_inner, LV_OBJ_FLAG_SCROLLABLE);

    // Add button
    lv_obj_t *btn_add = lv_btn_create(page_editor);
    lv_obj_set_size(btn_add, 70, 40);
    lv_obj_set_pos(btn_add, 405, 5);
    lv_obj_add_event_cb(btn_add, PopupManager::openAddCommand, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_add = lv_label_create(btn_add);
    lv_label_set_text(lbl_add, "Add");
    lv_obj_center(lbl_add);

    // Remove button
    lv_obj_t *btn_remove = lv_btn_create(page_editor);
    lv_obj_set_size(btn_remove, 70, 40);
    lv_obj_set_pos(btn_remove, 405, 50);
    lv_obj_add_event_cb(btn_remove, delete_selected_command, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_remove = lv_label_create(btn_remove);
    lv_label_set_text(lbl_remove, "Del");
    lv_obj_center(lbl_remove);

    // Up button
    lv_obj_t *btn_up = lv_btn_create(page_editor);
    lv_obj_set_size(btn_up, 70, 40);
    lv_obj_set_pos(btn_up, 405, 95);
    lv_obj_add_event_cb(btn_up, up_event, LV_EVENT_ALL, NULL);
    lv_obj_t *lbl_up = lv_label_create(btn_up);
    lv_label_set_text(lbl_up, "UP");
    lv_obj_center(lbl_up);

    // Down button
    lv_obj_t *btn_down = lv_btn_create(page_editor);
    lv_obj_set_size(btn_down, 70, 40);
    lv_obj_set_pos(btn_down, 405, 140);
    lv_obj_add_event_cb(btn_down, down_event, LV_EVENT_ALL, NULL);
    lv_obj_t *lbl_down = lv_label_create(btn_down);
    lv_label_set_text(lbl_down, "Down");
    lv_obj_center(lbl_down);
    
    // Options button
    lv_obj_t *btn_options = lv_btn_create(page_editor);
    lv_obj_set_size(btn_options, 70, 40);
    lv_obj_set_pos(btn_options, 405, 185);
    lv_obj_add_event_cb(btn_options, PopupManager::openOptions, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_options = lv_label_create(btn_options);
    lv_label_set_text(lbl_options, "OPT");
    lv_obj_center(lbl_down);
}


////////////////////////////
// --- Initialization --- //
////////////////////////////


void Setup_lvgl_selector() {
    build_home_page();
    build_editor_page();
    build_diag_page();
    show_page(page_home); // Start on auton page
}
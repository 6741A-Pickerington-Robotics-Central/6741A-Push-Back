#ifndef POPUP_HPP
#define POPUP_HPP
#include "robot/ui/commandbar.hpp"
#include "robot/ui/colors.hpp"
#include "robot/robot.hpp"
#include "liblvgl/lvgl.h"   
extern lv_obj_t* list_inner;

void goto_home(lv_event_t *e);
void save_auton_event(lv_event_t* e);
void highlight_selected();
void render_visible_range();
void ensure_selected_visible();
extern int selected_index;
extern std::vector<CommandData> command_list;

/**
 * @brief Popup for setting any number.
 *
 * Shows a numpad and allows the user to type any number including doubles.
 */
class NumberKeyPopup {
private:
    static inline lv_obj_t* active_button = nullptr;
    static inline char number_input[32] = {0};

    // === Event handlers (same as your global ones) ===
    static void number_key_event(lv_event_t* e) {
        lv_obj_t* display_label = (lv_obj_t*)lv_event_get_user_data(e);
        const char* key = lv_label_get_text(lv_obj_get_child(lv_event_get_target(e), 0));
        const char* old = lv_label_get_text(display_label);
        std::string updated = std::string(old) + key;
        lv_label_set_text(display_label, updated.c_str());
    }

    static void dot_key_event(lv_event_t* e) {
        lv_obj_t* display_label = (lv_obj_t*)lv_event_get_user_data(e);
        const char* old = lv_label_get_text(display_label);
        if (!strchr(old, '.')) {
            std::string updated = std::string(old) + ".";
            lv_label_set_text(display_label, updated.c_str());
        }
    }

    static void backspace_event(lv_event_t* e) {
        lv_obj_t* display_label = (lv_obj_t*)lv_event_get_user_data(e);
        std::string current = lv_label_get_text(display_label);
        if (!current.empty()) {
            current.pop_back();
            lv_label_set_text(display_label, current.c_str());
        }
    }

    static void enter_event(lv_event_t* e) {
        if (active_button) {
            lv_obj_t* display_label = (lv_obj_t*)lv_event_get_user_data(e);
            const char* value = lv_label_get_text(display_label);
            lv_label_set_text(lv_obj_get_child(active_button, 0), value);
            // Persist change to command_list
            lv_obj_t* bar = lv_obj_get_parent(active_button);
            if (bar) {
                intptr_t idx = (intptr_t)lv_obj_get_user_data(bar);
                if (idx >= 0 && idx < (intptr_t)command_list.size()) {
                    // regenerate the serialized line from the live bar and store
                    command_list[(size_t)idx].line = serialize_bar(bar);
                }
            }
        }
        lv_obj_t* btn_enter = lv_event_get_current_target(e);
        lv_obj_t* popup = lv_obj_get_parent(btn_enter);
        lv_obj_del(popup); // delete popup
        active_button = nullptr;
    }
public:
    // === Main function: opens popup ===
    static void open(lv_event_t* e) {
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
        if (strcmp(number_input, "0") == 0) {
            number_input[0] = '\0';
            lv_label_set_text(display_label, "");
        } else {
            lv_label_set_text(display_label, number_input);
        }
        lv_obj_set_pos(display_label, 10, 0);

        // Manually create buttons 1–9
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
        lv_obj_add_event_cb(btn_enter, enter_event, LV_EVENT_CLICKED, display_label);
    }
};

/**
 * @brief Popup for setting speed.
 *
 * Shows a 270° arc and allows the user to select a speed value from 0-270.
 */
class NumberSpeedPopup {
private:
    static inline lv_obj_t* active_button = nullptr;
    static inline char number_input[32] = {0};
    static inline lv_obj_t* display_label = nullptr;
    // Enter button event
    static void enter_event(lv_event_t* e) {
    if (active_button) {
        lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);
        const char* value = lv_label_get_text(label);
        lv_label_set_text(lv_obj_get_child(active_button, 0), value);
        // persist
        lv_obj_t* bar = lv_obj_get_parent(active_button);
        if (bar) {
            intptr_t idx = (intptr_t)lv_obj_get_user_data(bar);
            if (idx >= 0 && idx < (intptr_t)command_list.size()) {
                command_list[(size_t)idx].line = serialize_bar(bar);
            }
        }
    }
    lv_obj_t* btn_enter = lv_event_get_target(e);
    lv_obj_t* popup = lv_obj_get_parent(btn_enter);
    lv_obj_del(popup);
    active_button = nullptr;
}

public:
    static void open(lv_event_t* e) {
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
        // Display label
        display_label = lv_label_create(popup);
        lv_label_set_text(display_label, number_input);
        lv_obj_align(display_label, LV_ALIGN_CENTER, 0, -20);
        // Arc
        lv_obj_t* arc = lv_arc_create(popup);
        lv_obj_set_size(arc, 120, 120);
        lv_obj_align(arc, LV_ALIGN_CENTER, 0, -20);
        lv_arc_set_range(arc, 0, 270);
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);
        // Arc callback
        lv_obj_add_event_cb(arc, [](lv_event_t* e) {
            lv_obj_t* arc = lv_event_get_target(e);
            int value = lv_arc_get_value(arc);
            snprintf(number_input, sizeof(number_input), "%d", value);
            lv_label_set_text(display_label, number_input);
        }, LV_EVENT_VALUE_CHANGED, display_label);
        // Initialize arc value from number_input
        int initial_value = atoi(number_input);
        lv_arc_set_value(arc, initial_value);
        // Enter button
        lv_obj_t* btn_enter = lv_btn_create(popup);
        lv_obj_set_size(btn_enter, 150, 40);
        lv_obj_align(btn_enter, LV_ALIGN_CENTER, 0, 60);
        lv_obj_t* lbl_enter = lv_label_create(btn_enter);
        lv_label_set_text(lbl_enter, "Enter");
        lv_obj_center(lbl_enter);
        lv_obj_add_event_cb(btn_enter, enter_event, LV_EVENT_CLICKED, display_label);
    }
};

/**
 * @brief Popup for setting directions.
 *
 * Shows a 360° arc and allows the user to select a direction value.
 */
class NumberDirPopup {
private:
    static inline lv_obj_t* active_button = nullptr;
    static inline char number_input[32] = {0};
    static inline lv_obj_t* display_label = nullptr;
    // Enter button event
    static void enter_event(lv_event_t* e) {
    if (active_button) {
        lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);
        const char* value = lv_label_get_text(label);
        lv_label_set_text(lv_obj_get_child(active_button, 0), value);
        // persist
        lv_obj_t* bar = lv_obj_get_parent(active_button);
        if (bar) {
            intptr_t idx = (intptr_t)lv_obj_get_user_data(bar);
            if (idx >= 0 && idx < (intptr_t)command_list.size()) {
                command_list[(size_t)idx].line = serialize_bar(bar);
            }
        }
    }
    lv_obj_t* btn_enter = lv_event_get_target(e);
    lv_obj_t* popup = lv_obj_get_parent(btn_enter);
    lv_obj_del(popup);
    active_button = nullptr;
}

public:
    static void open(lv_event_t* e) {
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
        // Display label
        display_label = lv_label_create(popup);
        lv_label_set_text(display_label, number_input);
        lv_obj_align(display_label, LV_ALIGN_CENTER, 0, -25);
        // Arc
        lv_obj_t* arc = lv_arc_create(popup);
        lv_obj_set_size(arc, 120, 120);
        lv_obj_align(arc, LV_ALIGN_CENTER, 0, -25);
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        // Full 360-degree direction arc
        lv_arc_set_range(arc, -180, 180);
        lv_arc_set_rotation(arc, 90);  // Start from top (12 o'clock)
        lv_arc_set_bg_angles(arc, 0, 360);
        lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        // Update label when arc changes
        lv_obj_add_event_cb(arc, [](lv_event_t* e) {
            lv_obj_t* arc = lv_event_get_target(e);
            int value = lv_arc_get_value(arc);
            snprintf(number_input, sizeof(number_input), "%d", value);
            lv_label_set_text(display_label, number_input);
        }, LV_EVENT_VALUE_CHANGED, display_label);
        // Initialize arc
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
        lv_obj_add_event_cb(btn_enter, enter_event, LV_EVENT_CLICKED, display_label);
    }
};

/**
 * @brief Popup for auton editor options like save and return to home screen.
 *
 * Shows a box on the side of the screen and allows the user to do some actions with the editor.
 */
class OptionsPopup {
private:
    static inline lv_obj_t* popup = nullptr;
    // Close button event
    static void close_event(lv_event_t* e) {
        if (popup) {
            lv_obj_del(popup);
            popup = nullptr;
        }
    }
    // Home button event
    static void go_home_close_event(lv_event_t* e) {
        if (popup) {
            lv_obj_del(popup);
            popup = nullptr;
        }
        goto_home(e);
    }
    // Save and run button event
    static void save_and_run_auton_event(lv_event_t* e) {
        save_auton_event(e);
        // Delete popup
        if (popup) {
            lv_obj_del(popup);
            popup = nullptr;
        }
        // Start auton in separate PROS task
        //pros::Task([](void*) {
        //    // Slight delay to allow user to move away
        //    pros::delay(1000);
        //    // Run the auton
        //    //();
        //}, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "RunAutonTask");
        pros::delay(1000);
        std::string path = get_auton_file_path(); // your existing function
        std::vector<std::string> auton_lines = load_auton_for_runtxt(path);
        runtxtauton(auton_lines);

    }
public:
    static void open(lv_event_t* e) {
        lv_obj_t* parent = lv_scr_act();
        popup = lv_obj_create(parent);
        lv_obj_set_size(popup, 160, 200);
        lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(popup, LV_ALIGN_CENTER, 80, 0);
        lv_obj_set_style_bg_color(popup, purple, 0);
        lv_obj_set_style_border_width(popup, 0, 0);
        lv_obj_set_style_border_color(popup, purple, 0);
        // Save button
        lv_obj_t* btn_save = lv_btn_create(popup);
        lv_obj_set_size(btn_save, 150, 40);
        lv_obj_align(btn_save, LV_ALIGN_CENTER, 0, -70);
        lv_obj_t* lbl_save = lv_label_create(btn_save);
        lv_label_set_text(lbl_save, "Save");
        lv_obj_center(lbl_save);
        lv_obj_add_event_cb(btn_save, save_auton_event, LV_EVENT_CLICKED, nullptr);
        // Run button
        lv_obj_t* btn_run = lv_btn_create(popup);
        lv_obj_set_size(btn_run, 150, 40);
        lv_obj_align(btn_run, LV_ALIGN_CENTER, 0, -23);
        lv_obj_t* lbl_run = lv_label_create(btn_run);
        lv_label_set_text(lbl_run, "Run");
        lv_obj_center(lbl_run);
        lv_obj_add_event_cb(btn_run, save_and_run_auton_event, LV_EVENT_CLICKED, nullptr);
        // Home button
        lv_obj_t* btn_home = lv_btn_create(popup);
        lv_obj_set_size(btn_home, 150, 40);
        lv_obj_align(btn_home, LV_ALIGN_CENTER, 0, 23);
        lv_obj_t* lbl_home = lv_label_create(btn_home);
        lv_label_set_text(lbl_home, "Home");
        lv_obj_center(lbl_home);
        lv_obj_add_event_cb(btn_home, go_home_close_event, LV_EVENT_CLICKED, nullptr);
        // Close button
        lv_obj_t* btn_close = lv_btn_create(popup);
        lv_obj_set_size(btn_close, 150, 40);
        lv_obj_align(btn_close, LV_ALIGN_CENTER, 0, 70);
        lv_obj_t* lbl_close = lv_label_create(btn_close);
        lv_label_set_text(lbl_close, "Close");
        lv_obj_center(lbl_close);
        lv_obj_add_event_cb(btn_close, close_event, LV_EVENT_CLICKED, nullptr);
    }
};

/**
 * @brief Popup for adding a command to the auton sequence.
 * 
 */
class AddCommandPopup {
private:
    static inline lv_obj_t* popup = nullptr;

    struct AsyncAddPayload {
        const char* type;
        int selected_index_snapshot;
    };

    // Called asynchronously by LVGL
    static void async_add_callback(void* p) {
    AsyncAddPayload* payload = static_cast<AsyncAddPayload*>(p);
    if (!payload) return;
    CommandData cmd;
    cmd.type = payload->type;
    // Fill default serialized line with zeros so it doesn’t get discarded
    if (cmd.type == "move") cmd.line = "m,r,0,0,0";
    else if (cmd.type == "wait") cmd.line = "w,r,0";
    else if (cmd.type == "motor") cmd.line = "s,a,0";
    else if (cmd.type == "piston") cmd.line = "p,d,0";
    else cmd.line = ""; // fallback, shouldn't happen
    // Determine insert index
    int insert_index = payload->selected_index_snapshot + 1;
    if (insert_index < 0) insert_index = 0;
    if (insert_index > (int)command_list.size()) insert_index = command_list.size();
    // Insert into model
    command_list.insert(command_list.begin() + insert_index, cmd);
    selected_index = insert_index;
    // Refresh visible bars
    ensure_selected_visible();
    // Delete popup
    if (popup && lv_obj_is_valid(popup)) {
        lv_obj_del(popup);
        popup = nullptr;
    }
    delete payload;
}

    static void add_command_event(lv_event_t* e) {
        const char* type = static_cast<const char*>(lv_event_get_user_data(e));
        if (!type) return;

        AsyncAddPayload* payload = new AsyncAddPayload;
        payload->type = type;
        payload->selected_index_snapshot = selected_index;

        // Schedule the add asynchronously
        lv_async_call(async_add_callback, payload);
    }

public:
    static void open() {
        // Delete any existing popup
        if (popup && lv_obj_is_valid(popup)) {
            lv_obj_del(popup);
            popup = nullptr;
        }
        lv_obj_t* parent = lv_scr_act();
        popup = lv_obj_create(parent);
        lv_obj_set_size(popup, 160, 190);
        lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(popup, LV_ALIGN_CENTER, 80, 0);
        lv_obj_set_style_bg_color(popup, purple, 0);
        lv_obj_set_style_border_width(popup, 0, 0);

        auto make_button = [&](const char* label, const char* type, int y) {
            lv_obj_t* btn = lv_btn_create(popup);
            lv_obj_set_size(btn, 140, 35);
            lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y);
            lv_obj_add_event_cb(btn, add_command_event, LV_EVENT_CLICKED, (void*)type);
            lv_obj_t* lbl = lv_label_create(btn);
            lv_label_set_text(lbl, label);
            lv_obj_center(lbl);
        };
        make_button("Move",  "move",  0);
        make_button("Motor", "motor", 45);
        make_button("Wait",  "wait",  90);
        make_button("Piston",  "piston",  135);
    }
};


/**
 * @brief Popup manager for all popups.
 *
 * Provides static methods to open different types of popups:\n
 * \li `openNumberKey(lv_event_t* e)` : Opens a keypad popup.
 * \li `openSpeed(lv_event_t* e)`     : Opens a speed selector popup.
 * \li `openDir(lv_event_t* e)`       : Opens a direction selector popup.
 * \li `openOptions(lv_event_t* e)`   : Opens the options popup.\n
 *
 * Example usage:\n
 * \code
 * PopupManager::openNumberKey(event);
 * PopupManager::openOptions(event);
 * \endcode
 */
class PopupManager {
public:
    // Called by create_number_button as event cb; user_data is the source button
    static void openNumberKey(lv_event_t* e) {
        NumberKeyPopup::open(e);
    }
    // Open speed popup
    static void openSpeed(lv_event_t* e) {
        NumberSpeedPopup::open(e);
    }
    // Open direction popup
    static void openDir(lv_event_t* e) {
        NumberDirPopup::open(e);
    }
    // Open options popup
    static void openOptions(lv_event_t* e) {
        OptionsPopup::open(e);
    }
    // Open add command popup
    static void openAddCommand(lv_event_t* e) {
        AddCommandPopup::open();
    }
};
#endif // POPUP_HPP


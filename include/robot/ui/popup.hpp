#ifndef POPUP_HPP
#define POPUP_HPP
#include "robot/ui/commandbar.hpp"
#include "robot/ui/colors.hpp"
#include "liblvgl/lvgl.h"   
extern lv_obj_t* list_inner;

void goto_home(lv_event_t *e);
void save_auton_event(lv_event_t* e);
void highlight_selected();
void update_list_position();
extern int selected_index;

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
public:
    static void open(lv_event_t* e) {
        lv_obj_t* parent = lv_scr_act();
        popup = lv_obj_create(parent);
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
        lv_obj_add_event_cb(btn_save, save_auton_event, LV_EVENT_CLICKED, nullptr);
        // Home button
        lv_obj_t* btn_home = lv_btn_create(popup);
        lv_obj_set_size(btn_home, 150, 40);
        lv_obj_align(btn_home, LV_ALIGN_CENTER, 0, 20);
        lv_obj_t* lbl_home = lv_label_create(btn_home);
        lv_label_set_text(lbl_home, "Home");
        lv_obj_center(lbl_home);
        lv_obj_add_event_cb(btn_home, go_home_close_event, LV_EVENT_CLICKED, nullptr);
        // Close button
        lv_obj_t* btn_close = lv_btn_create(popup);
        lv_obj_set_size(btn_close, 150, 40);
        lv_obj_align(btn_close, LV_ALIGN_CENTER, 0, 65);
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
    static void add_command_event(const std::string& type) {
        // Create new CommandBar in the correct container
        CommandBar new_bar = CommandBar::create(list_inner, type);
        // Compute correct insertion index (after the selected bar)
        lv_obj_move_to_index(new_bar.bar, selected_index + 1);
        // Optionally refresh visuals
        highlight_selected();
        update_list_position();
        // Close popup
        if (popup) {
            lv_obj_del(popup);
            popup = nullptr;
        }
    }
    static void close_event(lv_event_t* e) {
        if (popup) {
            lv_obj_del(popup);
            popup = nullptr;
        }
    }
    static void add_move_cb(lv_event_t* e) {
        add_command_event("move");
    }
    static void add_wait_cb(lv_event_t* e) {
        add_command_event("wait");
    }
    static void add_motor_cb(lv_event_t* e) {
        add_command_event("motor");
    }
    static void add_piston_cb(lv_event_t* e) {
        add_command_event("piston");
    }

public:
    static void open(lv_obj_t* selected_bar) {
        lv_obj_t* parent = lv_scr_act();
        popup = lv_obj_create(parent);
        lv_obj_set_size(popup, 160, 190);
        lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(popup, LV_ALIGN_CENTER, 80, 0);
        lv_obj_set_style_bg_color(popup, purple, 0);
        lv_obj_set_style_border_width(popup, 0, 0);
        lv_obj_set_style_border_color(popup, purple, 0);

        lv_obj_t* btn_move = lv_btn_create(popup);
        lv_obj_set_size(btn_move, 140, 35);
        lv_obj_align(btn_move, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_add_event_cb(btn_move, add_move_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_t* lbl_move = lv_label_create(btn_move);
        lv_label_set_text(lbl_move, "Move");
        lv_obj_center(lbl_move);

        lv_obj_t* btn_motor = lv_btn_create(popup);
        lv_obj_set_size(btn_motor, 140, 35);
        lv_obj_align(btn_motor, LV_ALIGN_TOP_MID, 0, 45);
        lv_obj_add_event_cb(btn_motor, add_motor_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_t* lbl_motor = lv_label_create(btn_motor);
        lv_label_set_text(lbl_motor, "Motor");
        lv_obj_center(lbl_motor);
        
        
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
    // Open number key popup
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
        AddCommandPopup::open(list_inner);
    }
};
#endif // POPUP_HPP


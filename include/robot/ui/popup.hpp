#pragma once
#include "robot/ui/ui_main.hpp"

// =============================
// Base Class: PopupBase
// =============================
class PopupBase {
protected:
    lv_obj_t* popup = nullptr;
    lv_obj_t* display_label = nullptr;

public:
    virtual ~PopupBase() {
        if (popup) lv_obj_del(popup);
    }

    // Create popup contents
    virtual void create(lv_obj_t* parent) = 0;

    // Hide popup
    void close() {
        if (popup) {
            lv_obj_del(popup);
            popup = nullptr;
        }
    }

    lv_obj_t* get_obj() const { return popup; }
};

// =============================
// Helper: Add a simple button
// =============================
inline lv_obj_t* add_button(lv_obj_t* parent, const char* text, lv_event_cb_t callback, void* user_data = nullptr) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 50, 50);
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);
    lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, user_data);
    return btn;
}

// =============================
// Number Key Popup
// =============================

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

// =============================
// Speed Popup
// =============================

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

// =============================
// Dir Popup
// =============================

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

// =============================
// Options Popup (Dropdown)
// =============================
class OptionsPopup : public PopupBase {
private:
    lv_obj_t* active_button;
    const char* options;

    static void confirm_cb(lv_event_t* e) {
        auto self = static_cast<OptionsPopup*>(lv_event_get_user_data(e));
        lv_obj_t* dropdown = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        uint16_t selected = lv_dropdown_get_selected(dropdown);
        char buf[64];
        lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
        lv_label_set_text(lv_obj_get_child(self->active_button, 0), buf);
        self->close();
    }

public:
    OptionsPopup(lv_obj_t* button, const char* opts)
        : active_button(button), options(opts) {}

    void create(lv_obj_t* parent) override {
        popup = lv_obj_create(parent);
        lv_obj_set_size(popup, 200, 120);
        lv_obj_align(popup, LV_ALIGN_CENTER, 0, 0);
        lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(popup, purple, 0);

        lv_obj_t* dropdown = lv_dropdown_create(popup);
        lv_dropdown_set_options(dropdown, options);
        lv_obj_center(dropdown);

        lv_obj_t* confirm = add_button(popup, "✔", confirm_cb, this);
        lv_obj_align(confirm, LV_ALIGN_BOTTOM_MID, 0, -5);
    }
};

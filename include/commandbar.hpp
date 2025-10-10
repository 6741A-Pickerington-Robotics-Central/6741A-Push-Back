#pragma once
#include "liblvgl/lvgl.h"
#include <string>
#include <fstream>

// Optional forward declarations if you have them elsewhere
extern lv_color_t yellow;
extern lv_color_t green;
extern lv_color_t blue;
extern lv_color_t red;

// Forward declare helper function from your code
lv_obj_t* create_number_button(lv_obj_t* parent, const char* text, int id);

struct CommandBar {
    public:
    lv_obj_t* bar;        // Root LVGL object for this command
    std::string type;     // "move", "wait", "motor", or "piston"

    // === Factory ===
    static CommandBar create(lv_obj_t* parent, const std::string& type) {
        CommandBar cmd{};
        cmd.bar = lv_obj_create(parent);
        cmd.type = type;

        lv_obj_set_size(cmd.bar, 350, 55);
        lv_obj_set_flex_flow(cmd.bar, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_flex_cross_place(cmd.bar, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_all(cmd.bar, 4, 0);
        lv_obj_clear_flag(cmd.bar, LV_OBJ_FLAG_SCROLLABLE);

        if (type == "move") {
            lv_obj_set_style_bg_color(cmd.bar, lv_palette_main(LV_PALETTE_YELLOW), 0);
            lv_label_set_text(lv_label_create(cmd.bar), "Move to x,y:");
            create_number_button(cmd.bar, "0", 0);
            create_number_button(cmd.bar, "0", 0);
            lv_label_set_text(lv_label_create(cmd.bar), "dir:");
            create_number_button(cmd.bar, "0", 2);
        }
        else if (type == "wait") {
            lv_obj_set_style_bg_color(cmd.bar, lv_palette_main(LV_PALETTE_GREEN), 0);
            lv_label_set_text(lv_label_create(cmd.bar), "Wait");
            create_number_button(cmd.bar, "0", 0);
            lv_label_set_text(lv_label_create(cmd.bar), "sec");
        }
        else if (type == "motor") {
            lv_obj_set_style_bg_color(cmd.bar, lv_palette_main(LV_PALETTE_BLUE), 0);
            lv_label_set_text(lv_label_create(cmd.bar), "Spin");
            lv_obj_t* motor = lv_dropdown_create(cmd.bar);
            lv_dropdown_set_options(motor, "Intake Top\nIntake Middle\nIntake Bottom");
            lv_label_set_text(lv_label_create(cmd.bar), "at");
            create_number_button(cmd.bar, "0", 1);
        }
        else if (type == "piston") {
            lv_obj_set_style_bg_color(cmd.bar, lv_palette_main(LV_PALETTE_RED), 0);
            lv_label_set_text(lv_label_create(cmd.bar), "Toggle");
            lv_obj_t* piston = lv_dropdown_create(cmd.bar);
            lv_dropdown_set_options(piston, "Descore\nWeedwacker");
            lv_label_set_text(lv_label_create(cmd.bar), "to");
            lv_switch_create(cmd.bar);
        }

        return cmd;
    }

    // === Serialize (convert LVGL state -> string line) ===
    std::string serialize() const {
        if (type == "move") {
            lv_obj_t* btnX = lv_obj_get_child(bar, 1);
            lv_obj_t* btnY = lv_obj_get_child(bar, 2);
            lv_obj_t* btnDir = lv_obj_get_child(bar, 4);
            const char* x = lv_label_get_text(lv_obj_get_child(btnX, 0));
            const char* y = lv_label_get_text(lv_obj_get_child(btnY, 0));
            const char* dir = lv_label_get_text(lv_obj_get_child(btnDir, 0));
            return "m,r," + std::string(x) + "," + std::string(y) + "," + std::string(dir);
        }
        else if (type == "wait") {
            lv_obj_t* btn = lv_obj_get_child(bar, 1);
            const char* sec = lv_label_get_text(lv_obj_get_child(btn, 0));
            return "w,r," + std::string(sec);
        }
        else if (type == "motor") {
            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* btn = lv_obj_get_child(bar, 3);
            const char* speed = lv_label_get_text(lv_obj_get_child(btn, 0));
            char buffer[32];
            lv_dropdown_get_selected_str(dropdown, buffer, sizeof(buffer));
            char motor_letter = 'a';
            if (strcmp(buffer, "Intake Top") == 0) motor_letter = 'a';
            else if (strcmp(buffer, "Intake Middle") == 0) motor_letter = 'b';
            else if (strcmp(buffer, "Intake Bottom") == 0) motor_letter = 'c';
            return "s," + std::string(1, motor_letter) + "," + std::string(speed);
        }
        else if (type == "piston") {
            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* sw = lv_obj_get_child(bar, 3);
            char buffer[32];
            lv_dropdown_get_selected_str(dropdown, buffer, sizeof(buffer));
            char piston_letter = 'd';
            if (strcmp(buffer, "Descore") == 0) piston_letter = 'd';
            else if (strcmp(buffer, "Weedwacker") == 0) piston_letter = 'w';
            int state = lv_obj_has_state(sw, LV_STATE_CHECKED) ? 1 : 0;
            return "p," + std::string(1, piston_letter) + "," + std::to_string(state);
        }
        return "";
    }

    // === Deserialize (load string data -> LVGL UI) ===
    void deserialize(const std::string& line) {
        if (type == "move") {
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
        }
        else if (type == "wait") {
            std::string sec = line.substr(4);
            lv_obj_t* btn = lv_obj_get_child(bar, 1);
            lv_label_set_text(lv_obj_get_child(btn, 0), sec.c_str());
        }
        else if (type == "motor") {
            char motor_letter = line[2];
            std::string speed = line.substr(4);
            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* btn = lv_obj_get_child(bar, 3);
            if (motor_letter == 'a') lv_dropdown_set_selected(dropdown, 0);
            else if (motor_letter == 'b') lv_dropdown_set_selected(dropdown, 1);
            else if (motor_letter == 'c') lv_dropdown_set_selected(dropdown, 2);
            lv_label_set_text(lv_obj_get_child(btn, 0), speed.c_str());
        }
        else if (type == "piston") {
            char piston_letter = line[2];
            std::string state_str = line.substr(4);
            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* sw = lv_obj_get_child(bar, 3);
            if (piston_letter == 'd') lv_dropdown_set_selected(dropdown, 0);
            else if (piston_letter == 'w') lv_dropdown_set_selected(dropdown, 1);
            if (state_str == "1") lv_obj_add_state(sw, LV_STATE_CHECKED);
            else lv_obj_clear_state(sw, LV_STATE_CHECKED);
        }
    }

    private:
    // === UI builder based on type ===
    void build() {
        if (type == "move") {
            lv_obj_set_style_bg_color(bar, yellow, 0);
            lv_label_set_text(lv_label_create(bar), "Move to x,y:");
            create_number_button(bar, "0", 0);
            create_number_button(bar, "0", 0);
            lv_label_set_text(lv_label_create(bar), "dir:");
            create_number_button(bar, "0", 2);
        }
        else if (type == "wait") {
            lv_obj_set_style_bg_color(bar, green, 0);
            lv_label_set_text(lv_label_create(bar), "Wait");
            create_number_button(bar, "0", 0);
            lv_label_set_text(lv_label_create(bar), "sec");
        }
        else if (type == "motor") {
            lv_obj_set_style_bg_color(bar, blue, 0);
            lv_label_set_text(lv_label_create(bar), "Spin");
            auto motor = lv_dropdown_create(bar);
            lv_dropdown_set_options(motor, "Intake Top\nIntake Middle\nIntake Bottom");
            lv_label_set_text(lv_label_create(bar), "at");
            create_number_button(bar, "0", 1);
        }
        else if (type == "piston") {
            lv_obj_set_style_bg_color(bar, red, 0);
            lv_label_set_text(lv_label_create(bar), "Toggle");
            auto piston = lv_dropdown_create(bar);
            lv_dropdown_set_options(piston, "Descore\nWeedwacker");
            lv_label_set_text(lv_label_create(bar), "to");
            lv_switch_create(bar);
        }
    }

    // === Helper: Split string by comma ===
    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        for (char c : s) {
            if (c == delimiter) {
                if (!token.empty()) tokens.push_back(token);
                token.clear();
            } else token += c;
        }
        if (!token.empty()) tokens.push_back(token);
        return tokens;
    }

    // === Helper: Set text on number button safely ===
    void setButtonText(int index, const std::string& text) const {
        lv_obj_t* btn = lv_obj_get_child(bar, index);
        if (btn) {
            lv_obj_t* label = lv_obj_get_child(btn, 0);
            if (label) lv_label_set_text(label, text.c_str());
        }
    }
};
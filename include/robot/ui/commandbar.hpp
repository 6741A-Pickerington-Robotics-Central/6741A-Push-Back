#ifndef COMMANDBAR_HPP
#define COMMANDBAR_HPP
#include "robot/ui/colors.hpp"
// General headers
#include "pros/motors.h"
#include "pros/rtos.hpp"
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "liblvgl/lvgl.h"

// Forward Function Declarations
extern lv_obj_t* create_number_button(lv_obj_t* parent, const char* text, int id);
extern void update_command_list_from_bar(lv_obj_t* bar);

struct CommandData {
    std::string type;
    std::string line; // serialized command
};

// Return a serialized string for the lv_obj_t* bar (same format as CommandBar::serialize)

static std::string serialize_bar(lv_obj_t* bar) {
    if (!bar) return "";
    printf("Serializing bar...\n");
    // Determine type by background color like your save_auton_event did
    lv_color_t bg = lv_obj_get_style_bg_color(bar, 0);
    uint32_t bg32 = lv_color_to32(bg);
    uint32_t yellow = lv_color_to32(lv_palette_main(LV_PALETTE_YELLOW));
    uint32_t green  = lv_color_to32(lv_palette_main(LV_PALETTE_GREEN));
    uint32_t blue   = lv_color_to32(lv_palette_main(LV_PALETTE_BLUE));
    uint32_t red    = lv_color_to32(lv_palette_main(LV_PALETTE_RED));
    if (bg32 == yellow) {
        // move: layout was: label, btnX, btnY, label(dir:), btnDir
        lv_obj_t* btnX = lv_obj_get_child(bar, 1);
        lv_obj_t* btnY = lv_obj_get_child(bar, 2);
        lv_obj_t* btnDir = lv_obj_get_child(bar, 4);
        const char* x = lv_label_get_text(lv_obj_get_child(btnX, 0));
        const char* y = lv_label_get_text(lv_obj_get_child(btnY, 0));
        const char* dir = lv_label_get_text(lv_obj_get_child(btnDir, 0));
        return "m,r," + std::string(x) + "," + std::string(y) + "," + std::string(dir);
    } else if (bg32 == green) {
        // wait: label, btnSec, label sec
        lv_obj_t* btn = lv_obj_get_child(bar, 1);
        const char* sec = lv_label_get_text(lv_obj_get_child(btn, 0));
        return "w,r," + std::string(sec);
    } else if (bg32 == blue) {
        // motor: label, dropdown, label "at", btnSpeed
        lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
        lv_obj_t* btn = lv_obj_get_child(bar, 3);
        char buffer[32];
        lv_dropdown_get_selected_str(dropdown, buffer, sizeof(buffer));
        char motor_letter = ' ';
        if (strcmp(buffer, "Intake Top") == 0) motor_letter = 'a';
        else if (strcmp(buffer, "Intake Middle") == 0) motor_letter = 'b';
        else if (strcmp(buffer, "Intake Bottom") == 0) motor_letter = 'c';
        const char* speed = lv_label_get_text(lv_obj_get_child(btn, 0));
        return "s," + std::string(1, motor_letter) + "," + std::string(speed);
    } else if (bg32 == red) {
        // piston: label, dropdown, label "to", switch
        lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
        lv_obj_t* sw = lv_obj_get_child(bar, 3);
        char buffer[32];
        lv_dropdown_get_selected_str(dropdown, buffer, sizeof(buffer));
        char piston_letter = ' ';
        if (strcmp(buffer, "Descore") == 0) piston_letter = 'd';
        else if (strcmp(buffer, "Weedwacker") == 0) piston_letter = 'w';
        else if (strcmp(buffer, "Ball Block") == 0) piston_letter = 'b';
        int state = lv_obj_has_state(sw, LV_STATE_CHECKED) ? 1 : 0;
        printf("Piston save State: %d\n", state);
        return "p," + std::string(1, piston_letter) + "," + std::to_string(state);
    }
    return "";
}
/**
 * @brief A command bar representing a single robot action.
 *
 * This class encapsulates the LVGL objects and logic for a command bar,
 * which can represent different types of robot commands such as move, wait,
 * motor control, or piston control. It provides methods to create, serialize,
 * and deserialize the command bar state.
 */
class CommandBar {
public:
    lv_obj_t* bar;        // Root LVGL object for this command
    std::string type;     // "move", "wait", "motor", or "piston"

    // === Factory ===
    static CommandBar create(lv_obj_t* parent, const std::string& type) {
        CommandBar cmd;
        cmd.type = type;
        cmd.bar = lv_obj_create(parent);

        lv_obj_set_size(cmd.bar, 370, 55);
        lv_obj_set_flex_flow(cmd.bar, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_flex_cross_place(cmd.bar, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_all(cmd.bar, 4, 0);
        lv_obj_clear_flag(cmd.bar, LV_OBJ_FLAG_SCROLLABLE);

        // Use helper to build bar contents
        cmd.build();
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
            char motor_letter = ' ';
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
            char piston_letter = ' ';
            if (strcmp(buffer, "Descore") == 0) piston_letter = 'd';
            else if (strcmp(buffer, "Weedwacker") == 0) piston_letter = 'w';
            else if (strcmp(buffer, "Ball Block") == 0) piston_letter = 'b';
            int state = lv_obj_has_state(sw, LV_STATE_CHECKED) ? 1 : 0;
            return "p," + std::string(1, piston_letter) + "," + std::to_string(state);
        }
        return "";
    }

    // === Deserialize (load string data -> LVGL UI) ===
    void deserialize(const std::string& line) {
        if (line.size() < 3) return; // prevent out-of-range

        if (type == "move") {
            // Expected format: m,r,<x>,<y>,<dir>
            size_t pos1 = line.find(',', 4);
            size_t pos2 = (pos1 != std::string::npos) ? line.find(',', pos1 + 1) : std::string::npos;
            size_t pos3 = (pos2 != std::string::npos) ? line.find(',', pos2 + 1) : std::string::npos;

            if (pos1 == std::string::npos || pos2 == std::string::npos)
                return; // malformed line, skip safely

            std::string x = (line.size() > 4) ? line.substr(4, pos1 - 4) : "0";
            std::string y = (pos2 > pos1 + 1) ? line.substr(pos1 + 1, pos2 - pos1 - 1) : "0";
            std::string dir = (pos3 != std::string::npos && pos3 + 1 < line.size())
                                ? line.substr(pos2 + 1)
                                : "0";

            setButtonText(1, x);
            setButtonText(2, y);
            setButtonText(4, dir);
        }
        else if (type == "wait") {
            std::string sec = (line.size() > 4) ? line.substr(4) : "0";
            setButtonText(1, sec);
        }
        else if (type == "motor") {
            if (line.size() < 4) return;
            char motor_letter = line[2];
            std::string speed = (line.size() > 4) ? line.substr(4) : "0";

            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* btn = lv_obj_get_child(bar, 3);
            if (dropdown) {
                if (motor_letter == 'a') lv_dropdown_set_selected(dropdown, 0);
                else if (motor_letter == 'b') lv_dropdown_set_selected(dropdown, 1);
                else if (motor_letter == 'c') lv_dropdown_set_selected(dropdown, 2);
            }
            if (btn) lv_label_set_text(lv_obj_get_child(btn, 0), speed.c_str());
        }
        else if (type == "piston") {
            if (line.size() < 4) return;
            char piston_letter = line[2];
            std::string state_str = (line.size() > 4) ? line.substr(4) : "0";

            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* sw = lv_obj_get_child(bar, 3);
            if (dropdown) {
                if (piston_letter == 'd') lv_dropdown_set_selected(dropdown, 0);
                else if (piston_letter == 'w') lv_dropdown_set_selected(dropdown, 1);
            }
            if (sw) {
                if (state_str == "1") lv_obj_add_state(sw, LV_STATE_CHECKED);
                else lv_obj_clear_state(sw, LV_STATE_CHECKED);
            }
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
            lv_obj_add_event_cb(motor, [](lv_event_t* e){
                lv_obj_t* bar = lv_obj_get_parent(lv_event_get_target(e));
                update_command_list_from_bar(bar);
            }, LV_EVENT_VALUE_CHANGED, nullptr);
            lv_label_set_text(lv_label_create(bar), "at");
            create_number_button(bar, "0", 1);
        }
        else if (type == "piston") {
            lv_obj_set_style_bg_color(bar, red, 0);
            lv_label_set_text(lv_label_create(bar), "Toggle");
            auto piston = lv_dropdown_create(bar);
            lv_dropdown_set_options(piston, "Descore\nWeedwacker\nBall Block");
            lv_label_set_text(lv_label_create(bar), "to");
            lv_obj_t* toggle = lv_switch_create(bar);
            lv_obj_add_event_cb(piston, [](lv_event_t* e){
                lv_obj_t* bar = lv_obj_get_parent(lv_event_get_target(e));
                update_command_list_from_bar(bar);
            }, LV_EVENT_VALUE_CHANGED, nullptr);
        
            lv_obj_add_event_cb(toggle, [](lv_event_t* e){
                lv_obj_t* bar = lv_obj_get_parent(lv_event_get_target(e));
                update_command_list_from_bar(bar);
            }, LV_EVENT_VALUE_CHANGED, nullptr);
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
#endif // COMMANDBAR_HPP
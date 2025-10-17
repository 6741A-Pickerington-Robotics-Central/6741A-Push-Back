#ifndef COMMANDBAR_HPP
#define COMMANDBAR_HPP
#include "robot/ui/colors.hpp"
#include "robot/ezlog.hpp"
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

enum class CommandType : uint8_t {
    MoveToPose,
    MoveToPoint,
    Turn,
    Wait,
    Motor,
    Piston,
    Unknown
};

// Serialize an lv_obj_t* bar to a string
std::string serialize_bar(lv_obj_t* bar) {
    LOG_INFO("Serializing bar...");
    CommandType type = static_cast<CommandType>(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(bar)));
    switch (type) {
        case CommandType::MoveToPose: {
            lv_obj_t* btnX = lv_obj_get_child(bar, 1);
            lv_obj_t* btnY = lv_obj_get_child(bar, 2);
            lv_obj_t* btnDir = lv_obj_get_child(bar, 4);
            const char* x = lv_label_get_text(lv_obj_get_child(btnX, 0));
            const char* y = lv_label_get_text(lv_obj_get_child(btnY, 0));
            lv_obj_t* toggle = lv_obj_get_child(bar, 5);
            int toggle_state = lv_obj_has_state(toggle, LV_STATE_CHECKED) ? 1 : 0;
            LOG_INFO("Serialized A Move To Pose Command");
            return "M,r," + std::string(x) + "," + std::string(y) + "," + std::to_string(toggle_state);
        }
        case CommandType::MoveToPoint: {
            lv_obj_t* btnX = lv_obj_get_child(bar, 1);
            lv_obj_t* btnY = lv_obj_get_child(bar, 2);
            lv_obj_t* toggle = lv_obj_get_child(bar, 4);
            const char* x = lv_label_get_text(lv_obj_get_child(btnX, 0));
            const char* y = lv_label_get_text(lv_obj_get_child(btnY, 0));
            int toggle_state = lv_obj_has_state(toggle, LV_STATE_CHECKED) ? 1 : 0;
            LOG_INFO("Serialized A Move To Point Command");
            return "P,r," + std::string(x) + "," + std::string(y) + "," + std::to_string(toggle_state);
        }
        case CommandType::Turn: {
            lv_obj_t* btnDir = lv_obj_get_child(bar, 1);
            const char* dir = lv_label_get_text(lv_obj_get_child(btnDir, 0));
            LOG_INFO("Serialized A Turn Command");
            return "t,r," + std::string(dir);
        }
        case CommandType::Wait: {
            lv_obj_t* btn = lv_obj_get_child(bar, 1);
            const char* sec = lv_label_get_text(lv_obj_get_child(btn, 0));
            LOG_INFO("Serialized A Wait Command");
            return "w,r," + std::string(sec);
        }
        case CommandType::Motor: {
            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* btn = lv_obj_get_child(bar, 3);
            char buffer[32];
            lv_dropdown_get_selected_str(dropdown, buffer, sizeof(buffer));
            char motor_letter = ' ';
            if (strcmp(buffer, "Intake Top") == 0) motor_letter = 'a';
            else if (strcmp(buffer, "Intake Middle") == 0) motor_letter = 'b';
            else if (strcmp(buffer, "Intake Bottom") == 0) motor_letter = 'c';
            const char* speed = lv_label_get_text(lv_obj_get_child(btn, 0));
            LOG_INFO("Serialized A Motor Command");
            return "s," + std::string(1, motor_letter) + "," + std::string(speed);
        }
        case CommandType::Piston: {
            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* sw = lv_obj_get_child(bar, 3);
            char buffer[32];
            lv_dropdown_get_selected_str(dropdown, buffer, sizeof(buffer));
            char piston_letter = ' ';
            if (strcmp(buffer, "Descore") == 0) piston_letter = 'd';
            else if (strcmp(buffer, "Weedwacker") == 0) piston_letter = 'w';
            else if (strcmp(buffer, "Ball Block") == 0) piston_letter = 'b';
            int state = lv_obj_has_state(sw, LV_STATE_CHECKED) ? 1 : 0;
            LOG_INFO("Serialized A Pistion Command");
            return "p," + std::string(1, piston_letter) + "," + std::to_string(state);
        }
        default:
            LOG_WARN("Blank Type");
            return "";
    }
}

static CommandType string_to_command_type(const std::string& typeStr) {
    if (typeStr == "movetopose") return CommandType::MoveToPose;
    if (typeStr == "movetopoint") return CommandType::MoveToPoint;
    if (typeStr == "turn")       return CommandType::Turn;
    if (typeStr == "wait")       return CommandType::Wait;
    if (typeStr == "motor")      return CommandType::Motor;
    if (typeStr == "piston")     return CommandType::Piston;
    return CommandType::Unknown;
}

/**
 * @brief A command bar representing a single robot action.
 *
 * This class encapsulates the LVGL objects and logic for a command bar,
 * which can represent different types of robot commands such as move, wait,
 * motor control, or piston control. It provides methods to create 
 * and deserialize the command bar state.
 */
class CommandBar {
public:
    lv_obj_t* bar;        // Root LVGL object for this command
    std::string type;     // "move", "wait", "motor", or "piston"

    
    // === Factory ===
    static CommandBar create(lv_obj_t* parent, const std::string& typeStr) {
        LOG_INFO("Creating a command bar");
        CommandBar cmd;
        cmd.type = typeStr;  // keep string for old code
        CommandType typeEnum = string_to_command_type(typeStr);
        cmd.bar = lv_obj_create(parent);

        lv_obj_set_user_data(cmd.bar, reinterpret_cast<void*>(static_cast<uintptr_t>(typeEnum)));

        lv_obj_set_size(cmd.bar, 370, 55);
        lv_obj_set_flex_flow(cmd.bar, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_flex_cross_place(cmd.bar, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_all(cmd.bar, 4, 0);
        lv_obj_clear_flag(cmd.bar, LV_OBJ_FLAG_SCROLLABLE);

        // Use helper to build bar contents
        cmd.build();
        return cmd;
    }

    // === Deserialize (load string data -> LVGL UI) ===
    void deserialize(const std::string& line) {
        if (line.size() < 3) return;

        if (type == "movetopose") {
            printf("d1\n");
            std::vector<std::string> tokens = split(line, ',');
            printf("d2\n");
            //if (tokens.size() < 5) return;
            setButtonText(1, tokens[2]);
            setButtonText(2, tokens[3]);
            setButtonText(4, tokens[4]);
            printf("d3\n");
            lv_obj_t* toggle = lv_obj_get_child(bar, 5);
            printf("d4\n");
            if (tokens[5] == "1") lv_obj_add_state(toggle, LV_STATE_CHECKED);
            else lv_obj_clear_state(toggle, LV_STATE_CHECKED);
            printf("d5\n");
        } else if (type == "movetopoint") {
            printf("d1\n");
            std::vector<std::string> tokens = split(line, ',');
            printf("d2\n");
            //if (tokens.size() < 5) return;
            setButtonText(1, tokens[2]);
            setButtonText(2, tokens[3]);
            printf("d3\n");
            lv_obj_t* toggle = lv_obj_get_child(bar, 4);
            printf("Tokens has %zu elements\n", tokens.size());
            printf("d4\n");
            if (toggle == nullptr) {
                printf("Error: toggle is null\n");
            } else {
                printf("toggle pointer: %p\n", toggle);
            }
            if (tokens[4] == "1") {
                printf("setting toggle to true\n");
                lv_obj_add_state(toggle, LV_STATE_CHECKED);
                printf("set toggle to true\n");
            }
            else {
                printf("setting toggle to false\n");
                lv_obj_clear_state(toggle, LV_STATE_CHECKED);
                printf("set toggle to false\n");
            }
            printf("d5\n");
        } else if (type == "turn") {
            std::vector<std::string> tokens = split(line, ',');
            if (tokens.size() < 3) return;
            setButtonText(1, tokens[2]);
        } else if (type == "wait") {
            std::vector<std::string> tokens = split(line, ',');
            if (tokens.size() < 3) return;
            setButtonText(1, tokens[2]);
        } else if (type == "motor") {
            std::vector<std::string> tokens = split(line, ',');
            if (tokens.size() < 3) return;
            char motor_letter = tokens[1][0];
            std::string speed = tokens[2];

            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* btn = lv_obj_get_child(bar, 3);
            if (dropdown) {
                if (motor_letter == 'a') lv_dropdown_set_selected(dropdown, 0);
                else if (motor_letter == 'b') lv_dropdown_set_selected(dropdown, 1);
                else if (motor_letter == 'c') lv_dropdown_set_selected(dropdown, 2);
            }
            if (btn) lv_label_set_text(lv_obj_get_child(btn, 0), speed.c_str());
        } else if (type == "piston") {
            std::vector<std::string> tokens = split(line, ',');
            if (tokens.size() < 3) return;
            char piston_letter = tokens[1][0];
            std::string state_str = tokens[2];

            lv_obj_t* dropdown = lv_obj_get_child(bar, 1);
            lv_obj_t* sw = lv_obj_get_child(bar, 3);
            if (dropdown) {
                if (piston_letter == 'd') lv_dropdown_set_selected(dropdown, 0);
                else if (piston_letter == 'w') lv_dropdown_set_selected(dropdown, 1);
                else if (piston_letter == 'b') lv_dropdown_set_selected(dropdown, 2);
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
        if (type == "movetopose") {
            lv_obj_set_style_bg_color(bar, orange, 0);
            lv_label_set_text(lv_label_create(bar), "Move to x,y:");
            create_number_button(bar, "0", 0);
            create_number_button(bar, "0", 0);
            lv_label_set_text(lv_label_create(bar), "dir:");
            create_number_button(bar, "0", 2);
            lv_label_set_text(lv_label_create(bar), "Forward:");
            lv_obj_t* toggle = lv_switch_create(bar);
            lv_obj_add_event_cb(toggle, [](lv_event_t* e){
                lv_obj_t* bar = lv_obj_get_parent(lv_event_get_target(e));
                update_command_list_from_bar(bar);
            }, LV_EVENT_VALUE_CHANGED, nullptr);
        } else if (type == "movetopoint") {
            lv_obj_set_style_bg_color(bar, yellow, 0);
            lv_label_set_text(lv_label_create(bar), "Move to x,y:");
            printf("b1\n");
            create_number_button(bar, "0", 0);
            create_number_button(bar, "0", 0);
            lv_label_set_text(lv_label_create(bar), "Forward:");
            printf("b2\n");
            lv_obj_t* toggle = lv_switch_create(bar);
            lv_obj_add_event_cb(toggle, [](lv_event_t* e){
                lv_obj_t* bar = lv_obj_get_parent(lv_event_get_target(e));
                update_command_list_from_bar(bar);
            }, LV_EVENT_VALUE_CHANGED, nullptr);
            printf("b3\n");
        } else if (type == "turn") {
            lv_obj_set_style_bg_color(bar, teal, 0);
            lv_label_set_text(lv_label_create(bar), "Turn to dir:");
            create_number_button(bar, "0", 2);
        } else if (type == "wait") {
            lv_obj_set_style_bg_color(bar, green, 0);
            lv_label_set_text(lv_label_create(bar), "Wait");
            create_number_button(bar, "0", 0);
            lv_label_set_text(lv_label_create(bar), "sec");
        } else if (type == "motor") {
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
        } else if (type == "piston") {
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
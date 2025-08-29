#include "selector.h"

std::vector<std::string> items; // Global vector to store loaded items
int selectedauton = 0; // Variable to store the selected autonomous mode
int selectedautontoedit = 0; // Variable to store the selected autonomous mode for editing
int selectedName = 0; // Variable to store the selected name for the autonomous mode
int sidecolor = 0; // Variable to store the color of the selected autonomous mode
int screen = 1; // Variable to track the current screen (1 for auton bar, 2 for auton editor)
bool screenUpdating = false; // Flag to prevent multiple screen updates
int saving = 0; // Variable to track the saving state (0 for not saving, 1 for saving, 2 for saved)
int keyboard = 1; // Variable to track the current keyboard layout (1 for numbers, 2 for drive letters, 3 for other letters)
int selectedline = 1; // Variable to track the selected line in the auton editor
int save_timer = 0; // Timer for saving state
int screencooldown = 0; // Cooldown for screen updates
pros::screen_touch_status_s_t status; // Variable to store touch status
int last_screen = 0; // Variable to store the last screen for debugging
bool running = false; // Flag to prevent multiple autons from running simultaneously
int autonCount = autonOptions.size(); // Variable to store the number of available autonomous routines

/////////////////////////////////////////////
//             Basic Functions             //
/////////////////////////////////////////////

bool button_press_at(int xpos1, int ypos1, int xpos2, int ypos2, int touch_status) {
    // Check if the screen is currently being pressed and the touch is within bounds
    if (debug) {
        if (screen == last_screen) { // Only draw if the screen hasn't changed
            set("pen","debug main"); // Set pen color for debugging
            pros::screen::draw_line(xpos1, ypos1, xpos2, ypos2); // Draw a red line for debugging
            set("pen","debug secondary"); // Set pen color for debugging
            pros::screen::draw_line(xpos2, ypos1, xpos1, ypos2); // Draw a red line for debugging
            set("pen","text main"); // Reset pen color to white
        }
    }
    if (status.touch_status == touch_status) { // Assuming 1 means "pressed"
        if (touch_status == 2) { // Check for hold
            screencooldown = 10; // Reset cooldown on hold
        } else if (touch_status == 1) { // Check for tap
            screencooldown = 5; // Reset cooldown on tap
        }
        
        return (status.x >= xpos1 && status.x <= xpos2 && status.y >= ypos1 && status.y <= ypos2);
    }
    return false;
}

void set(std::string pen, std::string color) {
    if (color == "bg main") {
        if (pen == "fill") pros::screen::set_eraser(bg_main);
        if (pen == "pen") pros::screen::set_pen(bg_main);
    }
    if (color == "button") {
        if (pen == "fill") pros::screen::set_eraser(button);
        if (pen == "pen") pros::screen::set_pen(button);
    }
    if (color == "bg bar") {
        if (pen == "fill") pros::screen::set_eraser(bg_bar);
        if (pen == "pen") pros::screen::set_pen(bg_bar);
    }
    if (color == "highlight") {
        if (pen == "fill") pros::screen::set_eraser(highlight);
        if (pen == "pen") pros::screen::set_pen(highlight);
    }
    if (color == "text main") {
        if (pen == "fill") pros::screen::set_eraser(text_main);
        if (pen == "pen") pros::screen::set_pen(text_main);
    }
    if (color == "text bar") {
        if (pen == "fill") pros::screen::set_eraser(text_bar);
        if (pen == "pen") pros::screen::set_pen(text_bar);
    }
    if (color == "highlight") {
        if (pen == "fill") pros::screen::set_eraser(highlight);
        if (pen == "pen") pros::screen::set_pen(highlight);
    }
    if (color == "highlight secondary") {
        if (pen == "fill") pros::screen::set_eraser(highlight_secondary);
        if (pen == "pen") pros::screen::set_pen(highlight_secondary);
    }
    if (color == "debug main") {
        if (pen == "fill") pros::screen::set_eraser(debug_main);
        if (pen == "pen") pros::screen::set_pen(debug_main);
    }
    if (color == "debug secondary") {
        if (pen == "fill") pros::screen::set_eraser(debug_secondary);
        if (pen == "pen") pros::screen::set_pen(debug_secondary);
    }
}

std::string generateFileName(const std::string& prefix, int counter) {
    char fileName[13]; // 8.3 format: max 8 chars + 1 for '.' + 3 chars + 1 for '\0'
    snprintf(fileName, sizeof(fileName), "%.8s%02d.TXT", prefix.c_str(), counter);
    return std::string(fileName);
}

void check_for_loop(void) {
    if (selectedauton <= -3) {
        selectedauton = autonCount-1;
    }
    if (selectedauton >= autonCount) {
        selectedauton = -2;
    }
}

/////////////////////////////////////////////
//          Auton Class Functions          //
/////////////////////////////////////////////

ADIWrapper* findADIByLetter(char letter) { // Function to find an ADIWrapper by its letter
    for (auto& device : adiDevices) {
        if (device.letter[0] == letter) {
            return &device;  // Return a pointer to the matching device
        }
    }
    return nullptr;  // Return null if not found
}

MotorWrapper* findMotorByLetter(char letter) { // Function to find a MotorWrapper by its letter
    for (auto& motor : motorDevices) {
        if (motor.letter[0] == letter) {
            return &motor;  // Return a pointer to the matching motor
        }
    }
    return nullptr;  // Return null if not found
}

int getAutonIndexByFileID(int fileID) {
    for (size_t i = 0; i < autonOptions.size(); i++) {
        if (autonOptions[i].getFileNumber() == fileID) return i;
    }
    return -1; // Error case
}

Auton getAutonByName(const std::string& name) {
    for (const auto& auton : autonOptions) {
        if (auton.getName() == name) return auton; // Return the matching auton
    }
    return Auton("Invalid", pros::Color::black); // Return an error indicator: an empty or invalid auton
}

/////////////////////////////////////////////
//     Keyboard Layout Fixer Functions     //
/////////////////////////////////////////////

void updateKeyboardLayout(KeyboardKey keyboard3[]) { // Function to update keyboard labels based on motor/ADI states
    for (int i = 0; i < 5; i++) { // Update labels for motors on the top row (keys 0-4)
        if (i < motorDevices.size()) { // Check if there's a motor at this index
            keyboard3[i].label = motorDevices[i].letter.c_str(); // If there's a motor at this index, set its letter as the label
        } else {
            keyboard3[i].label = ""; // If there's no motor, set the label to an empty string
        }
    }
    for (int i = 5; i < 10; i++) { // Update labels for ADIs on the bottom row (keys 5-9)
        if (i - 5 < adiDevices.size()) { // Check if there's an ADI at this index
            keyboard3[i].label = adiDevices[i - 5].letter.c_str(); // If there's an ADI at this index, set its letter as the label
        } else {
            keyboard3[i].label = ""; // If there's no ADI, set the label to an empty string
        }
    }
}

void updateKeyboardLayoutlayout(Key keyboard3[]) { // Function to update keyboard labels based on motor/ADI states
    for (int i = 0; i < 5; i++) { // Update labels for motors on the top row (keys 0-4)
        if (i < motorDevices.size()) { // Check if there's a motor at this index
            keyboard3[i].character = motorDevices[i].letter[0]; // If there's a motor at this index, set its first letter as the label
        } else keyboard3[i].character = ' '; // If there's no motor, set the label to an empty string
    }
    for (int i = 5; i < 10; i++) { // Update labels for ADIs on the bottom row (keys 5-9)
        if (i - 5 < adiDevices.size()) { // Check if there's an ADI at this index
            keyboard3[i].character = adiDevices[i - 5].letter[0]; // If there's an ADI at this index, set its letter as the label
        } else keyboard3[i].character = ' '; // If there's no ADI, set the label to an empty string
    }
}

/////////////////////////////////////////////
//             Edit The Files              //
/////////////////////////////////////////////

void removeLastCharacter(std::vector<std::string>& items, int index) {
    // Check if the index is valid
    if (index >= 0 && index < items.size()) {
        if (!items[index].empty()) { // Check if the string is not empty
            items[index].pop_back(); // Remove the last character
        } else {
            printf("Item at index %d is already empty.\n", index);
        }
    } else {
        printf("Invalid index: %d.\n", index);
    }
}

void addCharToItem(std::vector<std::string>& items, size_t index, char character) {
    // Check if the index exists
    if (index >= items.size()) {
        // Resize the vector to include the desired index
        items.resize(index + 1); // This will create empty strings for new indices
    }
    // Add the character to the specified index
    items[index] += character;
}

void insertNewLine(std::vector<std::string>& items, int selectedLineToEdit) {
  // Ensure the index is within a valid range
  if (selectedLineToEdit >= 0 && selectedLineToEdit <= items.size()) {
      items.insert(items.begin() + selectedLineToEdit, ""); // Insert an empty string at the selected line
  } else {
    printf("Invalid line index: %d.\n", selectedLineToEdit);
  }
}

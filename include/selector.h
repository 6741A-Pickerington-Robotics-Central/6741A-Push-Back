#ifndef SELECTOR_H
#define SELECTOR_H
#include "p_selector.h" // Include the public selector header for the auton selector
#include "main.h" // Include the main header for the controller and other components

extern pros::MotorGroup leftMotors; // Left motor group for drive control
extern pros::MotorGroup rightMotors; // Right motor group for drive control
extern lemlib::Chassis chassis; // Global chassis object for drive control

// Store multiple MotorWrapper objects
extern std::vector<MotorWrapper> motorDevices;

// Function to find a MotorWrapper by letter
MotorWrapper* findMotorByLetter(char letter);

// Store multiple ADIWrapper objects
extern std::vector<ADIWrapper> adiDevices;

// Function to find an ADIWrapper by letter
ADIWrapper* findADIByLetter(char letter);

struct KeyboardKey { // Define keyboard array dynamically
    int x, y;
    const char* label;
};

struct Key {
    int x, y;
    char character;
};

void update_screen(int update_mode);
void set(std::string pen, std::string color);
bool button_press_at(int xpos1, int ypos1, int xpos2, int ypos2, int touch_status);
std::string generateFileName(const std::string& prefix, int counter);
std::vector<std::string> loadtxtauton(std::string filename);
void savetxtofauton(const std::string& filename, const std::vector<std::string>& items);
void removeLastCharacter(std::vector<std::string>& items, int index);
void addCharToItem(std::vector<std::string>& items, size_t index, char character);
void insertNewLine(std::vector<std::string>& items, int selectedLineToEdit);
void runtxtauton(std::vector<std::string> list);
void check_for_loop(void);
void loadautonsettingsFromFile();
int getAutonIndexByFileID(int fileID);
void updateKeyboardLayout(KeyboardKey keyboard3[]);
void updateKeyboardLayoutlayout(Key keyboard3[]);

extern std::vector<std::string> items; // Global vector to store loaded items
extern int selectedauton; // Variable to store the selected autonomous mode
extern int selectedautontoedit; // Variable to store the selected autonomous mode for editing
extern int selectedName; // Variable to store the selected name for the autonomous mode
extern int sidecolor; // Variable to store the color of the selected autonomous mode
extern int screen; // Variable to track the current screen (1 for auton bar, 2 for auton editor)
extern bool screenUpdating; // Flag to prevent multiple screen updates
extern int saving; // Variable to track the saving state (0 for not saving, 1 for saving, 2 for saved)
extern int keyboard; // Variable to track the current keyboard layout (1 for numbers, 2 for drive letters, 3 for other letters)
extern int selectedline; // Variable to track the selected line in the auton editor
extern int save_timer; // Timer for saving state
extern int screencooldown; // Cooldown for screen updates
extern pros::screen_touch_status_s_t status; // Variable to store touch status
extern int last_screen; // Variable to store the last screen for debugging
extern bool running; // Flag to prevent multiple autons from running simultaneously
extern int autonCount; // Variable to store the number of available autonomous routines

#endif // SELECTOR_H
#ifndef P_SELECTOR_H
#define P_SELECTOR_H
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "pros/abstract_motor.hpp"
#include "pros/motors.hpp"

extern pros::MotorGroup leftMotors; // Left motor group for drive control
extern pros::MotorGroup rightMotors; // Right motor group for drive control
extern lemlib::Chassis chassis; // Global chassis object for drive control

class MotorWrapper {
public:
    pros::Motor motor;
    std::string letter;
    MotorWrapper(int port, pros::v5::MotorGears gearset, std::string letter) : motor(port, gearset), letter(letter) {} // Constructor
    void set_velocity(int velocity) { // Example function
        motor.move_velocity(velocity);
    }
}; 

class ADIWrapper {
public:
    pros::adi::AnalogOut adiDevice;
    std::string letter;

    // Constructor
    ADIWrapper(int port, std::string letter) : adiDevice(port), letter(letter) {}

    // Example function to read the ADI sensor
    void set_value(bool value) {
        adiDevice.set_value(value);
    }
};

/*
class Auton {
private:
    std::string name; // Name of the auton
    int fileNumber; // File number (e.g., 1 = A01.txt)
    int cornerID; // Field corner (e.g., 0 = Red Close, 1 = Red Far, etc.)
    std::vector<std::string> commands; // Cached commands loaded from file

public:
    // Constructor
    Auton(int file_num = -1, const std::string& auton_name = "", int corner = -1)
        : name(auton_name), fileNumber(file_num), cornerID(corner) {}

    // Name
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }

    // File number
    int getFileNumber() const { return fileNumber; }
    void setFileNumber(int num) { fileNumber = num; }

    // Corner
    int getCorner() const { return cornerID; }
    void setCorner(int corner) { cornerID = corner; }

    // Commands
    const std::vector<std::string>& getCommands() const { return commands; }
    void setCommands(const std::vector<std::string>& cmds) { commands = cmds; }

    // Load the auton file and populate metadata + commands
    bool loadFromFile() {
        std::string filename = "/usd/A" + (fileNumber < 10 ? std::string("0") : std::string("")) + std::to_string(fileNumber);
        FILE* file = fopen(filename.c_str(), "r");
        if (!file) return false;

        commands.clear();
        char buffer[300];
        int lineNum = 0;

        while (fgets(buffer, sizeof(buffer), file)) {
            std::string line(buffer);
            if (!line.empty() && line.back() == '\n') line.pop_back();
            if (line.empty()) continue;

            if (lineNum == 0) name = line;               // First line = name
            else if (lineNum == 1) cornerID = std::stoi(line); // Second line = corner
            else commands.push_back(line);               // Remaining lines = commands

            lineNum++;
        }

        fclose(file);
        return true;
    }

    // Save this auton back to file
    bool saveToFile() const {
        std::string filename = "/usd/A" + (fileNumber < 10 ? std::string("0") : std::string("")) + std::to_string(fileNumber);
        std::ofstream file(filename);
        if (!file) return false;

        file << name << "\n";
        file << cornerID << "\n";
        for (const auto& cmd : commands) file << cmd << "\n";

        file.close();
        return file.good();
    }
};
*/

extern pros::MotorGroup temp3; // Motors for temperature line 3
extern const char temp3name[];
extern pros::MotorGroup temp4; // Motors for temperature line 4
extern const char temp4name[];

//extern std::vector<Auton> autonOptions; // Define available autonomous routines

extern bool debug;
extern pros::Color bg_main;
extern pros::Color button;
extern pros::Color bg_bar;
extern pros::Color highlight;
extern pros::Color highlight_secondary;
extern pros::Color text_main;
extern pros::Color text_bar;
extern pros::Color debug_main;
extern pros::Color debug_secondary;

void Setup_lvgl_selector();
void runauton(void);
void selector(void);
void skills_auton();

#endif // P_SELECTOR_H
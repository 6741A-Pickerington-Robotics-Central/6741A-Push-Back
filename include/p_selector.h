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

class Auton {
    private:
        const char* name; // The name of the autonomous routine
        pros::Color color; // The text color for the auton name
        int fileNumber;  // This will hold the file number (e.g., 1 for A01.txt)
    public:
        // Constructor
        Auton(const char* auton_name, pros::Color auton_color)
            : name(auton_name), color(auton_color) {}
    
        // Getter for name
        const char* getName() const {
            return name;
        }
    
        // Getter for color
        pros::Color getColor() const {
            return color;
        }

        void setFileNumber(int number) {
            fileNumber = number;
        }
    
        int getFileNumber() const {
            return fileNumber;
        }
};

extern pros::MotorGroup temp3; // Motors for temperature line 3
extern const char temp3name[];
extern pros::MotorGroup temp4; // Motors for temperature line 4
extern const char temp4name[];

extern std::vector<Auton> autonOptions; // Define available autonomous routines

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

void runauton(void);
void selector(void);
void skills_auton();

#endif // P_SELECTOR_H
#include "main.h" // Include the main header for the PROS library
#include "p_selector.h" // Include the selector header for the auton selector
#include "pros/misc.hpp"
#include "liblvgl/lvgl.h"
#include <map>

void screenTaskFunction(); // Forward declaration of the screen task function

pros::Controller controller(pros::E_CONTROLLER_MASTER); // Controller
pros::MotorGroup rightMotors({16,18,-19}, pros::MotorGearset::blue); // left motor group
pros::MotorGroup leftMotors({-15,-13,11}, pros::MotorGearset::blue); // right motor group
pros::Imu imu(7); // Inertial Sensor
pros::Rotation horizontalEnc(4); // Horizontal tracking wheel encoder.
pros::Rotation verticalEnc(5); // Vertical tracking wheel encoder.
lemlib::TrackingWheel horizontal(&horizontalEnc, lemlib::Omniwheel::NEW_275, 1); // Horizontal tracking wheel.
lemlib::TrackingWheel vertical(&verticalEnc, lemlib::Omniwheel::NEW_275, 0); // Vertical tracking wheel.
pros::Gps gps(21); // GPS sensor
// Drivetrain settings
lemlib::Drivetrain drivetrain(&leftMotors, // left motor group
                              &rightMotors, // right motor group
                              12, // track width
                              lemlib::Omniwheel::NEW_325,
                              480, // drivetrain rpm
                              2 // horizontal drift
);
// Lateral motion controller
lemlib::ControllerSettings linearController(6, // proportional gain (kP)
                                            0, // integral gain (kI)
                                            0.1, // derivative gain (kD)
                                            0, // anti windup
                                            1, // small error range, in inches
                                            100, // small error range timeout, in milliseconds
                                            0, // large error range, in inches
                                            0, // large error range timeout, in milliseconds
                                            0 // maximum acceleration (slew)
);
// Angular motion controller
lemlib::ControllerSettings angularController(0.9, // proportional gain (kP)
                                             0, // integral gain (kI)
                                             0.1, // derivative gain (kD)
                                             0, // anti windup
                                             3, // small error range, in degrees (2)
                                             100, // small error range timeout, in milliseconds (100)
                                             0, // large error range, in degrees (5)
                                             0, // large error range timeout, in milliseconds (500)
                                             0 // maximum acceleration (slew)
);
// Sensors for odometry
lemlib::OdomSensors sensors(&vertical, // vertical tracking wheel
                            nullptr, // vertical tracking wheel 2, set to nullptr as we don't have a second one
                            &horizontal, // horizontal tracking wheel
                            nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            &imu // inertial sensor
);
// Input curve for throttle input during driver control
lemlib::ExpoDriveCurve throttleCurve(3, // joystick deadband out of 127
                                     10, // minimum output where drivetrain will move out of 127
                                     1.019 // expo curve gain
);
// Input curve for steer input during driver control
lemlib::ExpoDriveCurve steerCurve(3, // joystick deadband out of 127
                                  10, // minimum output where drivetrain will move out of 127
                                  1.019 // expo curve gain
);
// Create the chassis
lemlib::Chassis chassis(drivetrain, linearController, angularController, sensors, &throttleCurve, &steerCurve);

//////////////////////////////////
//        Other Settings        //
//////////////////////////////////

// Motor declarations
pros::Motor Intake1(-3, pros::v5::MotorGears::green);  // first
pros::Motor Intake2(10, pros::v5::MotorGears::green);  // middle
pros::Motor Intake3(8, pros::v5::MotorGears::green);  // top


// Digital outputs
pros::adi::DigitalOut weedwacker('A');
pros::adi::DigitalOut descore('B'); // Redirects balls to either top goal or hopper
bool weedwackerState = false; // State of the weedwacker
int weedwackercooldown = 0; // Cooldown timer for weedwacker toggle

//////////////////////////////////
//          Gps Stuff           //
//////////////////////////////////

lemlib::Pose gpsToLemlib(double gpsXmm, double gpsYmm, double gpsHeading) {
    // Convert mm to inches
    double xInches = gpsXmm / 25.4;
    double yInches = gpsYmm / 25.4;
    // Convert heading: GPS = clockwise, Lemlib = counterclockwise
    double heading = 360 - gpsHeading;
    if (heading >= 360) heading -= 360;
    return lemlib::Pose(xInches, yInches, heading);
}

//////////////////////////////////
//           Main Code          //
//////////////////////////////////

void skills_auton() {
    // Skills auton
    chassis.moveToPose(2,45,-90,10000);
    pros::delay(1000);
    weedwacker.set_value(true);
    descore.set_value(true);
    pros::delay(100);
    chassis.moveToPose(-7,45,-90,10000);

}

void initialize() {
    chassis.calibrate(); // Calibrate sensors
    bool pidtuning = false; // Set to true to enable the PID tuning screen
    bool runonstart = false; // Set to true to run the selected auton on start
    if (pidtuning) pros::Task screenTask(screenTaskFunction); // Start the screen task for debugging
    else Setup_lvgl_selector(); // Setup the LVGL based auton selector
    if (runonstart) skills_auton(); // Run the selected auton if runonstart is true
}

void autonomous() {
  //runauton();
  //chassis.setPose(0, 0, 0); // Set position to x:0, y:0, heading:0
  chassis.setPose(gpsToLemlib(gps.get_position_x(), gps.get_position_y(), gps.get_heading())); // Set position to GPS reading
}

void opcontrol() {
    while (true) {
        // Drivetrain Controls
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
        int rightY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        chassis.tank(rightY, leftY);
        // Intake Controls
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_B)) { // Intake High Goal
            Intake1.move_velocity(200);
            Intake2.move_velocity(200);
            Intake3.move_velocity(-200);
        } else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y)) { // Intake Middle Goal
            Intake1.move_velocity(200);
            Intake2.move_velocity(200);
            Intake3.move_velocity(200);
        } else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN)) { // Outake
            Intake1.move_velocity(-200);
            Intake2.move_velocity(-200);
            Intake3.move_velocity(-200);
        } else { // Stop Intake
            Intake1.move_velocity(0);
            Intake2.move_velocity(0);
            Intake3.move_velocity(0);
        }
        // Weedwacker Controls
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            weedwacker.set_value(1);
        }
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            weedwacker.set_value(0);
        }
        // Descore and Ball Block Controls
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
            descore.set_value(true); // Descore and Ball Block Active
        }
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
            descore.set_value(false); // Descore and Ball Block Deactive
        }
        pros::delay(20); // Delay to save system resources
    }
}

void screenTaskFunction() {
    pros::lcd::initialize(); // Initialize the LCD
    pros::lcd::set_text(0, "Chassis Debug"); // Set the LCD title
    while (true) {
        pros::lcd::print(1, "Pose: (%.2f, %.2f, %.2f)", chassis.getPose().x, chassis.getPose().y, chassis.getPose().theta);
        pros::lcd::print(2, "Heading: %.2f", imu.get_heading());
        pros::lcd::print(3, "Horizontal Encoder: %i", horizontalEnc.get_position());
        pros::lcd::print(4, "Vertical Encoder: %i", verticalEnc.get_position());
        //lemlib::telemetrySink()->info("Chassis pose: {}", chassis.getPose());
        std::cout << "H: " << horizontalEnc.get_position() << " V: " << verticalEnc.get_position() << " Chassis Theta: " << chassis.getPose().theta << " IMU: " << imu.get_heading() << "\n";
        pros::delay(75);// Delay to save resources
    }
}
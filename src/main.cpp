#include "robot/robot.hpp" // Refrences to entire robot
#include "robot/ezlog.hpp"

void screenTaskFunction(); // Forward declaration of the screen task function

//////////////////////////////////
//           Main Code          //
//////////////////////////////////

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
  //chassis.moveToPose(50, 50, 90, 10000);
  //pros::delay(300);
  //chassis.moveToPose(0, 0, 0, 10000, {.forwards = false});
}

void opcontrol() {
    while (true) {
        if (!autonrunning) {
            // Drivetrain Controls
            int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
            int rightY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
            chassis.tank(rightY, leftY);
            // Intake Controls
            if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_B)) { // Intake High Goal
                Intake1.move_velocity(200);
                if (ballblockstate)
                {
                    Intake2.move_velocity(170);
                    Intake3.move_velocity(-200);
                } else {
                    Intake2.move_velocity(200);
                    Intake3.move_velocity(-200);
                }
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
                ballblock.set_value(true); // Descore and Ball Block Active
                ballblockstate = true;
            }
            if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
                ballblock.set_value(false); // Descore and Ball Block Deactive
                ballblockstate = false;
            }
            if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_A) && descoreCooldown <= 0) { // Toggle Descore Position with cooldown
                if (descoreState == false) {
                    descore.set_value(1); // Descore Active
                    descoreState = true;
                    descoreCooldown = 300; // 300ms cooldown
                } else {
                descore.set_value(0); // Descore Deactive
                descoreState = false;
                descoreCooldown = 300; // 300ms cooldown
                }
            } else {
                if (descoreCooldown > 0) descoreCooldown -= 20; // Decrease cooldown timer
            }
        }
        pros::delay(20); // Delay to save system resources
    }
}

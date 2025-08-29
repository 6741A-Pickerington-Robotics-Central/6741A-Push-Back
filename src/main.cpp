#include "main.h" // Include the main header for the PROS library
#include "p_selector.h" // Include the selector header for the auton selector
#include "pros/misc.hpp"

void screenTaskFunction(); // Forward declaration of the screen task function

pros::Controller controller(pros::E_CONTROLLER_MASTER); // Controller
pros::MotorGroup leftMotors({16,18,-19},pros::MotorGearset::blue); // left motor group - ports 3 (reversed), 4, 5 (reversed)
pros::MotorGroup rightMotors({-15,-13,11}, pros::MotorGearset::blue); // right motor group - ports 6, 7, 9 (reversed)
pros::Imu imu(16); // Inertial Sensor on port 16
pros::Rotation horizontalEnc(15); // Horizontal tracking wheel encoder.
pros::Rotation verticalEnc(11); // Vertical tracking wheel encoder.
lemlib::TrackingWheel horizontal(&horizontalEnc, lemlib::Omniwheel::NEW_275, 1); // Horizontal tracking wheel.
lemlib::TrackingWheel vertical(&verticalEnc, lemlib::Omniwheel::NEW_275, 2); // Vertical tracking wheel.
// Drivetrain settings
lemlib::Drivetrain drivetrain(&leftMotors, // left motor group
                              &rightMotors, // right motor group
                              12.5, // 10 inch track width
                              lemlib::Omniwheel::NEW_325, // using new 4" omnis
                              480, // drivetrain rpm is 360
                              2 // horizontal drift is 2. If we had traction wheels, it would have been 8
);
// Lateral motion controller
lemlib::ControllerSettings linearController(40, // proportional gain (kP)
                                            0.1, // integral gain (kI)
                                            1, // derivative gain (kD)
                                            0, // anti windup
                                            0, // small error range, in inches
                                            0, // small error range timeout, in milliseconds
                                            0, // large error range, in inches
                                            0, // large error range timeout, in milliseconds
                                            0 // maximum acceleration (slew)
);
// Angular motion controller
lemlib::ControllerSettings angularController(9, // proportional gain (kP)
                                             0, // integral gain (kI)
                                             45, // derivative gain (kD)
                                             0, // anti windup
                                             0, // small error range, in degrees
                                             0, // small error range timeout, in milliseconds
                                             0, // large error range, in degrees
                                             0, // large error range timeout, in milliseconds
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
pros::Motor Intake1(7, pros::v5::MotorGears::green);  // 3 chain/lower
pros::Motor Intake2(4, pros::v5::MotorGears::green);  // lower roller
pros::Motor Intake3(1, pros::v5::MotorGears::green);  // upper roller
pros::Motor Intake4(21, pros::v5::MotorGears::green);  // 2 chain/top


// Digital outputs
pros::adi::DigitalOut Weedwacker('A');
pros::adi::DigitalOut Kicker('B');
bool weedwackerState = false; // State of the weedwacker
int weedwackercooldown = 0; // Cooldown timer for weedwacker toggle

//Auton Editor and Selector Settings

bool debug = true; // Debug flag for screen interactions

// Define available autonomous routines
std::vector<Auton> autonOptions = { // No More than 10 Characters in the names
    Auton("Red Close", pros::Color::red),
    Auton("Red Far", pros::Color::red),
    Auton("Blue Close", pros::Color::blue),
    Auton("Blue Far", pros::Color::blue),
    Auton("Ex. Auton", pros::Color::black)
};

pros::Color bg_main = pros::Color::purple; //Main background color
pros::Color button = pros::Color::black; //Button background color
pros::Color bg_bar = pros::Color::white; //Selector top and bottom bar backgrounds
pros::Color highlight = pros::Color::red; //Highlight color in editor
pros::Color highlight_secondary = pros::Color::orange; //Highlight color in name selector
pros::Color text_main = pros::Color::white; //Main text color
pros::Color text_bar = pros::Color::purple; //Text color in the Selector Bars
pros::Color debug_main = pros::Color::red; //Debug Color 1
pros::Color debug_secondary = pros::Color::orange; // Debug Color 2

std::vector<MotorWrapper> motorDevices = { // Global list of MotorWrapper objects. MAX 5 MOTORs (port, gearset, "name")
    MotorWrapper (-13, pros::v5::MotorGears::blue, "i"), // Intake
    MotorWrapper (-10, pros::v5::MotorGears::blue, "r"), // Roller
    MotorWrapper (-14, pros::v5::MotorGears::green, "l"), // LadyBrown
    MotorWrapper (99, pros::v5::MotorGears::green, " "), // Blank
    MotorWrapper (99, pros::v5::MotorGears::green, " ") // Blank
};

std::vector<ADIWrapper> adiDevices = { // Global list of ADIWrapper objects. MAX 5 ADIs ('port',"name")
    ADIWrapper('B',"c"), // Clamping mechanism
    ADIWrapper('C',"m"), // Another device
    ADIWrapper('D',"p"), // Piston control
    ADIWrapper('z'," "), // Blank
    ADIWrapper('z'," ") // Blank
};

// Motor Temps for lines 3 and 4. It will avrage the temps of the the motors in each group
const char temp3name[] = "Intake:";
pros::MotorGroup temp3({-13,-10}); // Motors for temperature line 3
const char temp4name[] = "LadyBrown:";
pros::MotorGroup temp4({-14,15}); // Motors for temperature line 4

//////////////////////////////////
//           Main Code          //
//////////////////////////////////

void skills_auton() {
    // Skills auton
}

void initialize() {
    chassis.calibrate(); // Calibrate sensors
    //pros::Task screenTask(screenTaskFunction); // Start the screen task for debugging
    pros::Task selector_task(selector); // Run the auton selector in a separate task
    pros::delay(5000); // Wait for a moment to ensure everything is initialized
    //runauton(); 
}

void disabled() {}
void competition_initialize() {}

void autonomous() {
  runauton();
}

void opcontrol() {
    while (true) {
        // Get joystick positions
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
        chassis.tank(leftY, rightX);
        // Intake Controls     NEED TO BE MORE COMPLEX
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_B)) {
            Intake1.move_velocity(-200);
            Intake2.move_velocity(200);
            Intake3.move_velocity(-200);
            Intake4.move_velocity(-200);
        } else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y)) {
            Intake1.move_velocity(100);
            Intake2.move_velocity(-100);
            Intake3.move_velocity(100);
            Intake4.move_velocity(100);
        } else {
            Intake1.move_velocity(0);
            Intake2.move_velocity(0);
            Intake3.move_velocity(0);
            Intake4.move_velocity(0);
        }
        // Weedwacker Controls
        if (weedwackercooldown < 0) {
            if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_A)) {
                if (weedwackerState) Weedwacker.set_value(0);
                else Weedwacker.set_value(1);
                weedwackerState = !weedwackerState;
                weedwackercooldown = 20; // Set cooldown to prevent rapid toggling
            }
        } else {
            weedwackercooldown--;
        }
        
        // ____ Controls
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            //Kicker.set_value(1); // Engage kicker
        }
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            //Kicker.set_value(0); // Stop kicker
        }
        pros::delay(10); // delay to save resources
    }
}

void screenTaskFunction() {
    pros::lcd::initialize(); // Initialize the LCD
    pros::lcd::set_text(0, "Chassis Debug"); // Set the LCD title
    while (true) {
        pros::lcd::print(1, "Pose: (%.2f, %.2f, %.2f)", chassis.getPose().x, chassis.getPose().y, chassis.getPose().theta);
        pros::lcd::print(2, "Heading: %.2f", imu.get_heading());
        pros::lcd::print(3, "Horizontal Encoder: %d", horizontalEnc.get_position());
        pros::lcd::print(4, "Vertical Encoder: %d", verticalEnc.get_position());
        lemlib::telemetrySink()->info("Chassis pose: {}", chassis.getPose());
        pros::delay(50);// Delay to save resources
    }
}

/////////////////////////////////////////////////
//              pros terminal                  //
// Run the above line to connect to the brain. //
/////////////////////////////////////////////////
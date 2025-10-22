#include "robot/robot.hpp"
pros::Controller controller(pros::E_CONTROLLER_MASTER); // Controller
pros::MotorGroup rightMotors({16,18,-19}, pros::MotorGearset::blue); // left motor group
pros::MotorGroup leftMotors({11,-13,-15}, pros::MotorGearset::blue); // right motor group
pros::Imu imu(7); // Inertial Sensor
pros::Rotation horizontalEnc(4); // Horizontal tracking wheel encoder.
pros::Rotation verticalEnc(5); // Vertical tracking wheel encoder.
lemlib::TrackingWheel horizontal(&horizontalEnc, lemlib::Omniwheel::NEW_275, 1); // Horizontal tracking wheel.
lemlib::TrackingWheel vertical(&verticalEnc, lemlib::Omniwheel::NEW_275, 0); // Vertical tracking wheel.
// Drivetrain settings
lemlib::Drivetrain drivetrain(&leftMotors, // left motor group
                              &rightMotors, // right motor group
                              12, // track width
                              lemlib::Omniwheel::NEW_325,
                              480, // drivetrain rpm
                              2 // horizontal drift
);
// Lateral motion controller
lemlib::ControllerSettings linearController(8.5, // proportional gain (kP)
                                            0.01, // integral gain (kI)
                                            4, // derivative gain (kD)
                                            0, // anti windup
                                            1, // small error range, in inches
                                            500, // small error range timeout, in milliseconds
                                            2.5, // large error range, in inches
                                            1000, // large error range timeout, in milliseconds
                                            40 // maximum acceleration (slew)
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
pros::Motor Intake1(3, pros::v5::MotorGears::green);  // first
pros::Motor Intake2(10, pros::v5::MotorGears::green);  // middle
pros::Motor Intake3(8, pros::v5::MotorGears::green);  // top
//Intake1 = pros::Motor(3, pros::v5::MotorGears::green);  // first


// Digital outputs
pros::adi::DigitalOut weedwacker('A');
pros::adi::DigitalOut descore('B'); // Redirects balls to either top goal or hopper
pros::adi::DigitalOut ballblock('D'); // Blocks balls from exiting the intake
bool descoreState = false; // State of the weedwacker
int descoreCooldown = 0; // Cooldown timer for weedwacker toggle
bool ballblockstate = false; // State of the ball block

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
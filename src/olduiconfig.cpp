#if 0
//Auton Editor and Selector Settings

bool debug = true; // Debug flag for screen interactions

std::vector<Auton> autonOptions = { // No More than 10 Characters in the names
    Auton("Red Close"),
    Auton("Red Far"),
    Auton("Blue Close"),
    Auton("Blue Far"),
    Auton("Ex. Auton")
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
    ADIWrapper('z'," "), // Clamping mechanism
    ADIWrapper('z'," "), // Another device
    ADIWrapper('z'," "), // Piston control
    ADIWrapper('z'," "), // Blank
    ADIWrapper('z'," ") // Blank
};

// Motor Temps for lines 3 and 4. It will avrage the temps of the the motors in each group
const char temp3name[] = "Intake:";
pros::MotorGroup temp3({-13,-10}); // Motors for temperature line 3
const char temp4name[] = "LadyBrown:";
pros::MotorGroup temp4({-14,15}); // Motors for temperature line 4
#endif
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

//old ui code

#if 0

void printListToScreen(const std::vector<std::string>& items, int start_y, int y_increment, int linehighlight, int shift_highlight) {
    int y = start_y; // Initial y-coordinate for printing
    if (linehighlight >= shift_highlight) y -= y_increment * (linehighlight - shift_highlight); // Adjust y-coordinate if linehighlight is too high
    if (debug) {
        set("pen","debug_secondary");
        pros::screen::draw_line(2, start_y, 20, 200); // Debugging line to visualize the area
        pros::screen::draw_line(20, start_y, 20, y_increment * items.size() + start_y); // Debugging line to visualize the area
    }
    set("fill", "bg main");
    set("pen", "text main");
    for (int i = 0; i < items.size(); i++) {
        if (i == linehighlight) set("fill","highlight"); // Set color for highlighted text
        else set("fill","bg main"); // Default color
        if (y >= start_y) pros::screen::print(pros::E_TEXT_MEDIUM, 2, y, items[i].c_str()); // Print text at specified position
        y += y_increment;
    }
    set("fill", "bg main");
    set("pen", "text main");
}

void printAutonNames(int startX, int startY, int yOffset, int selectedIndex, int currentID, int shift_highlight) {
    int y = startY; // Initial y-coordinate for printing
    if (selectedIndex >= shift_highlight) y -= yOffset * (selectedIndex - shift_highlight); // Adjust y-coordinate if linehighlight is too high
    if (debug) {
        set("pen","debug_secondary");
        pros::screen::draw_line(2, startY, 20, 200); // Debugging line to visualize the area
        pros::screen::draw_line(20, startY, 20, yOffset * autonOptions.size() + startY); // Debugging line to visualize the area
    }
    for (size_t i = 0; i < autonOptions.size(); i++) {
        set("pen", "text main"); // Reset pen color to white
        if (i == selectedIndex) {
            set("fill","highlight"); // Highlight selected auton
            if (i == currentID) set("pen", "highlight secondary"); // Highlight if it matches the current name
        } else {
            if (i == currentID) {
                set("pen", "debug secondary"); // Highlight if it matches the current name
                set("fill", "bg main"); // Default color
            } else {
                set("fill", "bg main"); // Default color
                set("pen", "text main"); // Reset pen color to white
            }
        }
        pros::screen::print(pros::E_TEXT_LARGE, startX, y, autonOptions[i].getName()); // Print the auton name
        y += yOffset;
    }
}

void selector() {
    loadautonsettingsFromFile(); // Load the auton settings from file
    update_screen(0); // Initial screen update
    while (true) {
      if (screencooldown <= 0) {
        status = pros::screen::touch_status(); // Get the current touch status
        switch (screen) {
            case 1: {
                if (button_press_at(0, 50, 200, 200,2)) { // Back
                    selectedauton -= 1;
                    check_for_loop();
                    update_screen(0);
                }
                if (button_press_at(200 , 50, 482, 200,2)) { // Forward
                    selectedauton += 1;
                    check_for_loop();
                    update_screen(0);
                }
                if (button_press_at(0, 200, 200, 272,1)) { // Run Auton
                    pros::delay(1000); // Wait for a second before running the auton
                    runauton();
                }
                if (button_press_at(200, 200, 480, 272,1)) { // Auton Editor
                    screen = 2;
                    update_screen(0);
                }
                if (screencooldown <= -300) { // Refresh the screen if it hasn't been updated for a while
                    screencooldown = 0;
                    update_screen(1);
                }
                break;
            }
            case 2: {
                if (button_press_at(380, 0, 480, 33,1)) { // Clear Selection
                    for (int i = 0; i < autonCountOld; i++) {
                        if (selectedautontoedit == autonOptions[i].getFileNumber()) {  // Compare with the index
                            autonOptions[i].setFileNumber(-1); // Clear the auton file number
                            break;  // Exit loop after clearing the name
                        }
                    }
                    saveautonsettingsToFile(); // Save the auton settings to file
                    update_screen(1);
                }
                if (button_press_at(160, 0, 315, 33,1)) { // Set Name
                    screen = 4; // Go to color selection screen
                    update_screen(0);
                }
                if (button_press_at(185, 120, 310, 170,1)) { // Load
                    loadtxtauton(generateFileName("A", selectedautontoedit));
                    screen = 3;
                    selectedline = 0;
                    update_screen(0);
                }
                if (button_press_at(0, 55, 160, 190,2)) { // Back arrow
                    selectedautontoedit -= 1;
                    update_screen(1);
                }
                if (button_press_at(320, 55, 480, 190,2)) { // Forward arrow
                    selectedautontoedit += 1;
                    update_screen(1);
                }
                if (button_press_at(0, 195, 480, 272,1)) { // Back to selector
                    screen = 1;
                    update_screen(0);
                }
                break;
            }
            case 3: { // Auton Editor
                if (saving == 2) {
                    if (save_timer <= 0) {
                        saving = 0;
                        update_screen(0);
                    } else {
                        save_timer -= 1;
                    }
                }
                if (button_press_at(400, 0, 480, 32,1)) { // Save
                    saving = 1;
                    save_timer = 5;
                    update_screen(1);
                    savetxtofauton(generateFileName("A", selectedautontoedit), items);
                    printf("Save\n");
                }
                if (button_press_at(300, 0, 380, 32,1)) { // Open
                    screen = 2;
                    update_screen(0);
                    printf("Open\n");
                }
                if (button_press_at(220, 0, 280, 32,1)) { // Run
                    printf("Run\n");
                    pros::delay(1000);
                    chassis.calibrate(); // calibrate sensors
                    runtxtauton(loadtxtauton(generateFileName("A", selectedautontoedit)));
                    update_screen(0);
                }
                // Scroll buttons
                if (button_press_at(0, 30, 120, 130,2)) {
                    selectedline -= 1;
                    update_screen(3);
                }
                if (button_press_at(0, 131, 120, 260,2)) {
                    selectedline += 1;
                    update_screen(3);
                }
                // Keyboard interactions
                if (button_press_at(125, 188, 185, 249,1)) { // Switch keyboard layout
                    keyboard = (keyboard % 3) + 1;
                    update_screen(2);
                }
                if (button_press_at(200, 188, 261, 249,1)) { // "-" minus sign
                    addCharToItem(items, selectedline, '-');
                    update_screen(3);
                }
                if (button_press_at(275, 188, 336, 249,1)) { // "." peirod
                    addCharToItem(items, selectedline, '.');
                    update_screen(3);
                }
                if (button_press_at(350, 188, 411, 249, 1)) { // "," comma
                    addCharToItem(items, selectedline, ',');
                    update_screen(3);
                }
                if (button_press_at(425, 188, 486, 249,1)) { // Remove last character/Backspace
                    removeLastCharacter(items, selectedline);
                    update_screen(3);
                }
                Key keyboardLayout1[] = {
                    {125, 35, '1'}, {200, 35, '2'}, {275, 35, '3'}, {350, 35, '4'}, {425, 35, '5'},
                    {125, 110, '6'}, {200, 110, '7'}, {275, 110, '8'}, {350, 110, '9'}, {425, 110, '0'}
                };
                Key keyboardLayout2[] = {
                    {125, 35, 'd'}, {200, 35, 't'}, {275, 35, 'w'}, {350, 35, 'm'}, {425, 35, ' '},
                    {125, 110, 'y'}, {200, 110, ' '}, {275, 110, ' '}, {350, 110, ' '}, {425, 110, 'E'}
                };
                Key keyboardLayout3[] = {
                    {125, 35, 'i'}, {200, 35, 'c'}, {275, 35, 'p'}, {350, 35, 'k'}, {425, 35, 'l'},
                    {125, 110, 'a'}, {200, 110, ' '}, {275, 110, ' '}, {350, 110, ' '}, {425, 110, 'E'}
                };
                Key* layout;
                switch (keyboard) {
                    case 1: layout = keyboardLayout1; break;
                    case 2: layout = keyboardLayout2; break;
                    case 3: layout = keyboardLayout3; updateKeyboardLayoutlayout(keyboardLayout3); break;
                }
                for (int i = 0; i < 10; i++) {
                    if (button_press_at(layout[i].x, layout[i].y, layout[i].x + 60, layout[i].y + 61,1)) { // Check if the button is pressed
                        if (layout[i].character == 'E') { // Enter key
                            insertNewLine(items, selectedline);
                            update_screen(3);
                        } else if (layout[i].character == ' ') { // Blank key
                            // Do nothing for blank key
                        } else if (layout[i].character != ' ') { // If it's not a blank key, add the character to the item
                            addCharToItem(items, selectedline, layout[i].character);
                            update_screen(3);
                        }
                    }
                }
                break;
            }
            case 4: {
                if (button_press_at(0, 20, 300, 131,2)) { //Scroll up
                    selectedName -= 1;
                    update_screen(1);
                }
                if (button_press_at(0, 132, 300, 272,2)) { //Scroll down
                    selectedName += 1;
                    update_screen(1);
                }
                if (button_press_at(300, 20, 480, 131,1)) { //Select color
                    autonOptions[selectedName].setFileNumber(selectedautontoedit);
                    saveautonsettingsToFile(); // Save the auton settings to file
                    screen = 2;
                    update_screen(0);
                }
                if (button_press_at(300, 132, 480, 272,1)) { //Back to auton editor
                    screen = 2;
                    update_screen(0);
                }
            }
        }
    }
    screencooldown -= 1;
    last_screen = screen; // Update last_screen to the current screen
    pros::delay(50);
    }
}

void update_screen(int update_mode) {
  //BrainScreen brainscreen;
  if (screenUpdating) return; 
  screenUpdating = true; // Prevent multiple updates at once
  switch (screen) {
    case 1: {
        if (update_mode == 0) {
          set("pen","text main");
          pros::screen::fill_rect(0, 0, 480, 272); // Background
          // Auton Bar
          set("fill","bg bar"); // Draw background bar
          //pros::screen::fill_rect(0, 0, 480, 50); // Fill the top bar with the background color
          //brainscreen.draw_rect(0, 0, 480, 50); // Draw a rectangle in the middle for the auton name
          set("pen","text bar"); // Set text color and print "Auton:"
          pros::screen::print(pros::E_TEXT_LARGE, 0, 10, "Auton:");
          if (selectedauton >= 0 && selectedauton < autonCountOld) { // Determine which auton is selected and print its name
              Auton selected = autonOptions[selectedauton]; // Get the selected auton object
              pros::screen::set_pen(selected.getColor()); // Set the pen color based on the auton color
              pros::screen::print(pros::E_TEXT_LARGE, 130, 10, selected.getName()); // Print the auton name at the specified position
          } else if (selectedauton == -1) {
              pros::screen::set_pen(pros::Color::black); // Default color
              pros::screen::print(pros::E_TEXT_LARGE, 130, 10, "Skills");
              sidecolor = 0;
          } else if (selectedauton == -2) {
              pros::screen::set_pen(pros::Color::black); // Default color
              pros::screen::print(pros::E_TEXT_LARGE, 130, 10, "None");
              sidecolor = 0;
          } else {
              pros::screen::set_pen(pros::Color::black); // Default color
              pros::screen::print(pros::E_TEXT_LARGE, 130, 10, "Invalid Auton");
              sidecolor = 0;
          }
          // Run/No Auton Button
          set("pen","bg main");
          pros::screen::fill_rect(0, 200, 480, 50);
        }
        if (update_mode == 1 || update_mode == 0) {
          set("pen","text main");
          // Temperatures   
          float averageleftdrivetemp = leftMotors.get_temperature();
          float averagerightdrivetemp = rightMotors.get_temperature();
          float row3Temp = temp3.get_temperature();
          float row4Temp = temp4.get_temperature();
          // Print temperatures to the screen
          set("pen","text main");
          set("fill","bg main");
          pros::screen::print(pros::E_TEXT_LARGE, 0, 60, "Left Motors: %.1f", averageleftdrivetemp);
          pros::screen::print(pros::E_TEXT_LARGE, 0, 95, "Right Motors: %.1f", averagerightdrivetemp);
          pros::screen::print(pros::E_TEXT_LARGE, 0, 130, (temp3name + std::string(" %.1f")).c_str(), row3Temp);
          pros::screen::print(pros::E_TEXT_LARGE, 0, 165, (temp4name + std::string(" %.1f")).c_str(), row4Temp);
        }
        if (update_mode == 0) {
          set("pen","text bar");
          set("fill","bg bar"); 
          pros::screen::fill_rect(205, 200, 215, 250);  
          if (selectedauton == 6 || selectedauton == 7) {
              pros::screen::print(pros::E_TEXT_LARGE_CENTER, 10, 210, "No Auton");
          } else {
              pros::screen::print(pros::E_TEXT_LARGE_CENTER, 10, 210, "Run");
          }
          pros::screen::print(pros::E_TEXT_LARGE_CENTER, 230, 210, "Auton Editor");
        }
        break;
    }
    case 2: {
        if (update_mode == 0) {
          set("pen","bg main");
          // Background
          pros::screen::fill_rect(0, 0, 480, 272);
          // Auton Editor UI
          set("pen","text main");
          set("fill","button");
          pros::screen::print(pros::E_TEXT_MEDIUM, 0, 1, "Auton Editor");
          pros::screen::print(pros::E_TEXT_LARGE, 380, 1, "Clear");
          pros::screen::print(pros::E_TEXT_LARGE, 160, 1, "Set Name");
        }
        set("fill","button");
        if (update_mode == 1 || update_mode == 0) {
          // File Name
          std::string filename = generateFileName("A", selectedautontoedit);
          pros::screen::print(pros::E_TEXT_LARGE, 175, 80, filename.c_str());
          set("pen","bg main");
          pros::screen::fill_rect(145, 45, 345, 80);
          set("pen","text main");
          set("fill","bg main");
          for (int i = 0; i < autonCountOld; i++) {
            if (selectedautontoedit == autonOptions[i].getFileNumber()) {  // Compare with the index
                pros::screen::set_pen(autonOptions[i].getColor());  // Set the pen color based on the auton color
                pros::screen::print(pros::E_TEXT_LARGE, 145, 45, autonOptions[i].getName());
                break;  // Exit loop after printing the name
            }
          }              
        }
        if (update_mode == 0) {
          // Draw UI Elements
          set("pen","text main");
          pros::screen::draw_line(145, 179, 80, 114);
          pros::screen::draw_line(145, 51, 80, 116);
          pros::screen::draw_line(345, 179, 410, 114);
          pros::screen::draw_line(345, 51, 410, 116);   
          // Load Button
          pros::screen::print(pros::E_TEXT_LARGE, 200, 130, "LOAD");
          pros::screen::draw_rect(185, 120, 310, 170);  
          // Back to Selector
          pros::screen::print(pros::E_TEXT_LARGE, 90, 200, "Back to Selector");
          pros::screen::draw_line(0, 195, 480, 195);
        }
        break;
    }
    case 3: {
        set("pen","bg main");
        if (update_mode == 0) {
            // Background
            pros::screen::fill_rect(0, 0, 480, 272);
        }
        if (update_mode == 0 || update_mode == 3) {
            // Auton Editor UI
            pros::screen::fill_rect(0, 33, 120, 272);
            printListToScreen(items, 40, 20, selectedline, 7); // Print the list of items to the screen
        }
        if (update_mode == 0) {
            set("pen","text main");
            set("fill","button");
            pros::screen::print(pros::E_TEXT_MEDIUM, 1,1, "Auton Editor");
        }
        set("fill","button");
        set("pen","text main");
        if (update_mode == 1 || update_mode == 0) {
            // Background color switch based on saving state
            switch (saving) {
                case 0: pros::screen::set_eraser(pros::Color::black); break;
                case 1: pros::screen::set_eraser(pros::Color::orange); break;
                case 2: pros::screen::set_eraser(pros::Color::green); break;
            }
            // Save, Open, Run buttons
            pros::screen::print(pros::E_TEXT_LARGE, 400, 1, "Save");
        }
        set("fill","button");
        if (update_mode == 0) {
            pros::screen::print(pros::E_TEXT_LARGE, 300, 1, "Open");
            pros::screen::print(pros::E_TEXT_LARGE, 220, 1, "Run");
            // File name
            std::string filename = generateFileName("A", selectedautontoedit);
            pros::screen::print(pros::E_TEXT_MEDIUM, 1,18, filename.c_str());
            // Other keys
            struct Key {
                int x, y;
                const char* label;
            } keys[] = {
                {350, 180, ","}, {275, 180, "."}, {200, 180, "-"}, {425, 180, "B"}
            };
            for (const auto& key : keys) {
                set("pen","button");
                pros::screen::fill_rect(key.x, key.y, key.x + 60, key.y + 61);
                set("pen","text main");
                pros::screen::draw_rect(key.x, key.y, key.x + 60, key.y + 61);
                set("fill","button");
                pros::screen::print(pros::E_TEXT_LARGE, key.x + 20, key.y + 15, key.label);
            }
        }
        if (update_mode == 0 || update_mode == 2) {
            KeyboardKey keyboard1[] = { //Numbers
                {125, 35, "1"}, {200, 35, "2"}, {275, 35, "3"}, {350, 35, "4"}, {425, 35, "5"},
                {125, 110, "6"}, {200, 110, "7"}, {275, 110, "8"}, {350, 110, "9"}, {425, 110, "0"}
            };
            KeyboardKey keyboard2[] = { //Movement letters
                {125, 35, "m"}, {200, 35, "t"}, {275, 35, "s"}, {350, 35, ""}, {425, 35, ""},
                {125, 110, "w"}, {200, 110, ""}, {275, 110, ""}, {350, 110, ""}, {425, 110, "E"}
            };
            KeyboardKey keyboard3[] = { //Other motor and ADI letters
                {125, 35, ""}, {200, 35, ""}, {275, 35, ""}, {350, 35, ""}, {425, 35, ""},
                {125, 110, ""}, {200, 110, ""}, {275, 110, ""}, {350, 110, ""}, {425, 110, ""}
            };
            KeyboardKey* layout;
            const char* switchLabel = "";
            switch (keyboard) {
                case 1: layout = keyboard1; switchLabel = "Mov"; break;
                case 2: layout = keyboard2; switchLabel = "Oth"; break;
                case 3: layout = keyboard3; switchLabel = "123"; updateKeyboardLayout(keyboard3); break;
            }
            // Draw keyboard switch button
            set("pen","button");
            set("fill","button");
            pros::screen::fill_rect(125, 180, 185, 249);
            set("pen","text main");
            pros::screen::draw_rect(125, 180, 185, 249);
            pros::screen::print(pros::E_TEXT_LARGE, 125, 195, switchLabel);
            for (int i = 0; i < 10; i++) { // Draw keyboard keys
                set("pen","button");
                set("fill","button");
                pros::screen::fill_rect(layout[i].x, layout[i].y, layout[i].x + 60, layout[i].y + 61);
                set("pen","text main");
                pros::screen::draw_rect(layout[i].x, layout[i].y, layout[i].x + 60, layout[i].y + 61);
                pros::screen::print(pros::E_TEXT_LARGE, layout[i].x + 20, layout[i].y + 15, layout[i].label);
            }
        }
        break;
    }          
    case 4: {
        if (update_mode == 0) {
            // Background
            set("pen","bg main");
            pros::screen::fill_rect(0, 0, 480, 272);
            // Auton Corrner Selector
            set("pen","text main");
            set("fill","button");
            pros::screen::print(pros::E_TEXT_MEDIUM, 0,1, "Auton Editor");
            pros::screen::draw_rect(301, 0, 480, 131);
            pros::screen::draw_rect(301, 132, 480, 272);
            pros::screen::print(pros::E_TEXT_LARGE, 310, 150, "Back to");
            pros::screen::print(pros::E_TEXT_LARGE, 310, 190, "Editor");
            pros::screen::print(pros::E_TEXT_LARGE, 310, 30, "Set");
            pros::screen::print(pros::E_TEXT_LARGE, 310, 70, "Name");
        }
        if (update_mode == 1 || update_mode == 0) {
            set("pen","bg main");
            pros::screen::fill_rect(0, 20, 300, 272);
            set("fill","bg main");
            printAutonNames(10, 20, 50, selectedName, getAutonIndexByFileID(selectedautontoedit), 3); // Print the auton names to the screen      
        }
    }  
  }
  screenUpdating = false;
}

#endif

// old ui core functions

#if 0
std::vector<std::string> items; // Global vector to store loaded items
int selectedauton = 0; // Variable to store the selected autonomous mode
int selectedautontoedit = 0; // Variable to store the selected autonomous mode for editing
int selectedName = 0; // Variable to store the selected name for the autonomous mode
int sidecolor = 0; // Variable to store the color of the selected autonomous mode
int screen = 1; // Variable to track the current screen (1 for auton bar, 2 for auton editor)
bool screenUpdating = false; // Flag to prevent multiple screen updates
int saving = 0; // Variable to track the saving state (0 for not saving, 1 for saving, 2 for saved)
//int keyboard = 1; // Variable to track the current keyboard layout (1 for numbers, 2 for drive letters, 3 for other letters)
int selectedline = 1; // Variable to track the selected line in the auton editor
int save_timer = 0; // Timer for saving state
int screencooldown = 0; // Cooldown for screen updates
pros::screen_touch_status_s_t status; // Variable to store touch status
int last_screen = 0; // Variable to store the last screen for debugging
bool running = false; // Flag to prevent multiple autons from running simultaneously

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
        selectedauton = autonCountOld-1;
    }
    if (selectedauton >= autonCountOld) {
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

#endif
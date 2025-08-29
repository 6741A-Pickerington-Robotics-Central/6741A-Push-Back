#include "pros/colors.hpp"
#include "pros/motors.h"
#include "pros/rtos.hpp"
#include "pros/screen.hpp"
#include "selector.h"
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "screen.cpp"

/////////////////////////////////////////////
// Save and Load Autons and Auton Settings //
/////////////////////////////////////////////

void saveautonsettingsToFile() {
    // Open the file for writing
    std::ofstream file("/usd/autonsettings.txt");
    if (!file) {
        std::cerr << "Failed to open file for writing!\n";
        return;
    }
    // Write the index of each Auton to the file
    for (int i = 0; i < autonOptions.size(); i++) {
        // Assuming that autonOptions[i].getFileNumber() returns the file number (e.g., 1 for A01.txt)
        file << autonOptions[i].getFileNumber() << "\n";
    }
    file.close();
    if (file.good()) {
        std::cout << "Data saved successfully!\n";
    } else {
        std::cerr << "Failed to save data properly!\n";
    }
}

void loadautonsettingsFromFile() {
    std::string filepath = "/usd/autonsettings.txt"; // Construct the file path
    // Open the file for reading
    FILE* file = fopen(filepath.c_str(), "r");
    if (!file) {
        printf("File not found: %s\n", filepath.c_str());
        return;
    }
    // Buffer to store each line
    char buffer[100]; // Adjust size based on expected line length
    std::string line;
    int index = 0;
    // Read the file line by line
    while (fgets(buffer, sizeof(buffer), file)) {
        line = buffer;
        if (index < autonOptions.size()) {
            int fileNumber;
            std::stringstream(line) >> fileNumber;

            // Set the file number to the corresponding Auton object
            autonOptions[index].setFileNumber(fileNumber);  // Assuming setFileNumber is a setter for file number
            index++;
        }
    }
    fclose(file); // Close the file
    // Print the loaded values for debugging
    for (int i = 0; i < autonOptions.size(); i++) {
        std::cout << "Auton " << i + 1 << ": File number " << autonOptions[i].getFileNumber() << std::endl;
    }
}

void savetxtofauton(const std::string& filename, const std::vector<std::string>& items) {
    // Open the file for writing
    std::ofstream file("/usd/" + filename);

    if (!file) {
        std::cerr << "Failed to open file for writing!\n";

        return;
    }

    // Write each item to the file, separated by newlines
    for (const auto& item : items) {
        file << item << "\n";
    }

    file.close();
    
    if (file.good()) {
        std::cout << "Data saved to SD card successfully!\n";
        saving = 2;
        update_screen(1);
    } else {
        std::cerr << "Failed to save data properly!\n";
    }
}

std::vector<std::string> loadtxtauton(std::string filename) {
    // Clear existing items before loading new data
    items.clear();

    // Construct the file path
    std::string filepath = "/usd/" + filename;

    // Check if the file exists by attempting to open it
    FILE* file = fopen(filepath.c_str(), "r");
    if (!file) {
        printf("File not found: %s\n", filepath.c_str());
        return items;
    }

    // Read file line by line
    char buffer[300]; // Adjust size based on expected file size
    while (fgets(buffer, sizeof(buffer), file)) {
        std::string line(buffer);
        // Remove trailing newline character if present
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }
        // Avoid adding empty lines
        if (!line.empty()) {
            items.push_back(line);
        }
    }

    // Close the file
    fclose(file);

    // Print the decoded list for debugging
    printf("Decoded List:\n");
    for (const auto& item : items) {
        printf("%s\n", item.c_str());
    }

    return items;
}

/////////////////////////////////////////////
//             Run The Auton               //
/////////////////////////////////////////////

void runauton(void) {
    if (running) return; // Prevent running multiple autons simultaneously
    running = true; // Set the running flag to true
    leftMotors.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE); // Set left motors to brake mode
    rightMotors.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE); // Set right motors to brake mode
    chassis.setPose(0, 0, 0); // Set position to x:0, y:0, heading:0
    if (selectedauton >= 0 && selectedauton < autonOptions.size()) {
        int fileNumber = autonOptions[selectedauton].getFileNumber();
        runtxtauton(loadtxtauton(generateFileName("A", fileNumber)));
    } else if (selectedauton == -1) {
        skills_auton(); // Run the skills auton if selectedauton is -1
    } else if (selectedauton == -2) {
        pros::delay(1); //Do nothing
    }
    pros::delay(1000); // Allow IMU to stabilize
    leftMotors.set_brake_mode(pros::E_MOTOR_BRAKE_COAST); // Set left motors to coast mode
    rightMotors.set_brake_mode(pros::E_MOTOR_BRAKE_COAST); // Set right motors to coast mode
    running = false; // Set the running flag to false after the auton is complete
}

void runtxtauton(std::vector<std::string> list) {
    chassis.setPose(0, 0, 0); // Reset the chassis pose to (0, 0, 0) before running the auton
    if (list.empty()) {
      printf("The list is empty. No action taken.\n");
      return;
    }
    printf("Running a txt auton: %i\n", selectedautontoedit);
    // Iterate through the list
    for (const auto& item : list) {
      printf("Processing item: %s\n", item.c_str());  // Debugging line
      if (!item.empty()) {
        // Get the first letter of the current item
        char firstLetter = item[0];
        
        // Perform actions based on the first letter
        switch (firstLetter) {
            case 'm': {  // Handle 'm'
                printf("Processing '%s': First letter is 'm'. Performing move robot action.\n", item.c_str());
                const char* dataStr = item.c_str() + 1;  // Skip the first letter
            
                const char* firstComma = strchr(dataStr, ',');
                if (!firstComma) {
                    printf("Error: '%s' does not contain enough values.\n", item.c_str());
                    break;
                }
            
                const char* secondComma = strchr(firstComma + 1, ',');
                if (!secondComma) {
                    printf("Error: '%s' does not contain a third value.\n", item.c_str());
                    break;
                }
            
                char* endPtr;
                double x = std::strtod(dataStr, &endPtr);
                if (endPtr == dataStr || *endPtr != ',') {
                    printf("Error processing '%s': Could not extract a valid x.\n", item.c_str());
                    break;
                }
            
                double y = std::strtod(firstComma + 1, &endPtr);
                if (endPtr == firstComma + 1 || *endPtr != ',') {
                    printf("Error processing '%s': Could not extract a valid y.\n", item.c_str());
                    break;
                }
            
                double theta = std::strtod(secondComma + 1, &endPtr);
                if (endPtr == secondComma + 1) {
                    printf("Error processing '%s': Could not extract a valid theta.\n", item.c_str());
                    break;
                }
            
                printf("%f,%f,%f\n", x, y, theta);
                chassis.moveToPose(x, y, theta, 1000);
                break;
            }
            case 't': { // Handle 't'
            printf("Processing '%s': First letter is 't'. Performing turn robot action.\n", item.c_str());
            const char* dataStr = item.c_str() + 1;  // Skip the first letter
            double theta = std::strtod(dataStr, nullptr);  // Convert the remaining string to a double
            if (theta == 0.0 && dataStr[0] != '0') {
              printf("Error processing '%s': Could not convert to a valid theta.\n", item.c_str());
              break;
            }
            chassis.turnToHeading(theta, 1000); // Turn to the specified angle
            break;
            }  
            case 's': { // Handle 's'
                printf("Processing '%s': First letter is 's'. Performing swing robot action.\n", item.c_str());
                const char* dataStr = item.c_str() + 1;  // Skip the first letter
                double theta = std::strtod(dataStr, nullptr);  // Convert the remaining string to a double
                if (theta == 0.0 && dataStr[0] != '0') {
                  printf("Error processing '%s': Could not convert to a valid theta.\n", item.c_str());
                  break;
                }
                //chassis.swingToHeading(float theta, DriveSide lockedSide, int timeout); // Swing to the specified angle

            }
            default:  // Handle other cases
                ADIWrapper* matchingDevice = findADIByLetter(firstLetter);
                if (matchingDevice) {
                    printf("Processing '%s': Found matching ADIWrapper '%c'.\n", item.c_str(), firstLetter);
                    const char* numStr = item.c_str() + 1;  // Skip the first letter
                    float value = std::atof(numStr);  // Convert the remaining string to a float
                    if (value == 0.0 && numStr[0] != '0') {
                        printf("Error processing '%s': Could not convert to a valid float.\n", item.c_str());
                    } else {
                        matchingDevice->set_value(value != 0);
                    }
                } else {
                    printf("No matching ADIWrapper found for letter '%c'.\n", firstLetter);
                }
                MotorWrapper* matchingMotor = findMotorByLetter(firstLetter); // Search for a MotorWrapper with a matching letter      
                if (matchingMotor) {
                    printf("Processing '%s': Found matching MotorWrapper '%c'.\n", item.c_str(), firstLetter);

                    const char* numStr = item.c_str() + 1;  // Skip the first letter
                    int value = std::atoi(numStr);  // Convert the remaining string to an integer

                    if (value == 0 && numStr[0] != '0') {
                        printf("Error processing '%s': Could not convert to a valid integer.\n", item.c_str());
                    } else {
                        matchingMotor->set_velocity(value);
                    }
                } else {
                    printf("No matching MotorWrapper found for letter '%c'.\n", firstLetter);
                }
                break;
            }
      } else {
        printf("Skipping an empty item in the list.\n");
      }
    }
}  

/////////////////////////////////////////////
//      Selector And Screen Functions      //
/////////////////////////////////////////////

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
                    for (int i = 0; i < autonCount; i++) {
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
          screen.draw_rect(0, 0, 480, 50); // Draw a rectangle in the middle for the auton name
          set("pen","text bar"); // Set text color and print "Auton:"
          pros::screen::print(pros::E_TEXT_LARGE, 0, 10, "Auton:");
          if (selectedauton >= 0 && selectedauton < autonCount) { // Determine which auton is selected and print its name
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
          for (int i = 0; i < autonCount; i++) {
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
#include "robot/robot.hpp" // Refrences to entire robot
#include "robot/ezlog.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

bool autonrunning = false; // Flag to indicate if the auton is currently running

void driveForwardToTarget(double distance) {
    lemlib::Pose Pose = chassis.getPose();
    double headingRad = Pose.theta * M_PI / 180.0; // convert degrees → radians
    double newX = Pose.x + distance * sin(headingRad);
    double newY = Pose.y + distance * cos(headingRad);
    printf("Current Pose: x=%.2f, y=%.2f, theta=%.2f\n", Pose.x, Pose.y, Pose.theta);
    printf("New Target Pose: x=%.2f, y=%.2f, theta=%.2f\n", newX, newY, Pose.theta);
    if (distance >= 0) {
        chassis.moveToPoint(newX,newY, 1000); // same heading
    } else {
        chassis.moveToPoint(newX,newY, 1000, {.forwards = false}); // same heading
    }
}

void skills_auton() {
    autonrunning = true; // Set the running flag to true
    printf("Running Skills Auton!\n");
    chassis.setPose(0,0,0);
    chassis.moveToPoint(0,10,10000,{.forwards = true});
    // Skills auton
    //chassis.moveToPose(2,45,-90,10000);
    //pros::delay(1000);
    //weedwacker.set_value(true);
    //descore.set_value(true);
    //pros::delay(100);
    //chassis.moveToPose(-7,45,-90,10000);
    autonrunning = false; // Set the running flag to false after the auton is complete
}

std::vector<std::string> load_auton_for_runtxt(const std::string& filename) {
    std::vector<std::string> result;
    std::ifstream file(filename);
    if (!file.is_open()) {
        printf("Error: Could not open file %s\n", filename.c_str());
        return result; // empty
    }
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;
        // Check if line starts with a recognized command
        char type = line[0];
        if (type == 'M' || type == 'P'|| type == 't' || type == 'f' || type == 's' || type == 'w' || type == 'p') {
            // Basic validation: check commas exist
            size_t comma_count = std::count(line.begin(), line.end(), ',');
            if (comma_count >= 2) { // m,r,0,0,0 has 4 commas
                result.push_back(line);
            } else {
                printf("Warning: skipping malformed line: %s\n", line.c_str());
            }
        } else {
            printf("Warning: skipping unrecognized line: %s\n", line.c_str());
        }
    }
    file.close();
    return result;
}


void runauton() {
    if (autonrunning) return; // Prevent running multiple autons simultaneously
    chassis.setPose(0, 0, 0); // Set position to x:0, y:0, heading:0
    if (selected_corner == -1 || selected_slot == -1) {
        printf("No auton selected!\n");
    } else if (selected_corner == -2 && selected_slot == 0) {
        skills_auton(); // Run the skills auton if selected_corner is -2 and selected_slot is 0
    } else {
        std::string path = get_auton_file_path(); // your existing function
        std::vector<std::string> auton_lines = load_auton_for_runtxt(path);
        runtxtauton(auton_lines);
    }
    pros::delay(100); // Allow Robot to stabilize
}

void runtxtauton(const std::vector<std::string>& list) {
    autonrunning = true; // Set the running flag to true
    chassis.setPose(0, 0, 0); // Reset pose
    if (list.empty()) {
        printf("Auton list is empty!\n");
        return;
    }
    for (const auto& line : list) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        // Split CSV
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        if (tokens.empty()) continue;
        char type = tokens[0][0];        // 'M', 'P', 's', 'w', 'p'
        std::string device = (tokens.size() > 1) ? tokens[1] : "";
        lemlib::Pose currentPose = chassis.getPose();
        printf("Current Pose: x=%.2f, y=%.2f, theta=%.2f\n", currentPose.x, currentPose.y, currentPose.theta);
        switch (type) {
            case 'M': {
                //M,r,0,20,30,0
                if (tokens.size() < 6) {
                    printf("Error: malformed move command: %s\n", line.c_str());
                    break;
                }
                double x = std::stod(tokens[2]);
                double y = std::stod(tokens[3]);
                double theta = std::stod(tokens[4]);
                bool goforward = (tokens[5] == "0");
                printf("Move: x=%f, y=%f, theta=%f, forward:%d\n", x, y, theta, goforward);
                lemlib::MoveToPoseParams poseParams = {};
                poseParams.forwards = goforward;
                chassis.moveToPose(x, y, theta, 1000, poseParams);
                break;
            }
            case 'P': {
                if (tokens.size() < 5) {
                    printf("Error: malformed move command: %s\n", line.c_str());
                    break;
                }
                double x = std::stod(tokens[2]);
                double y = std::stod(tokens[3]);
                bool goforward = (tokens[4] == "0");
                printf("Move: x=%f, y=%f, backward=%d\n", x, y, goforward);
                lemlib::MoveToPointParams pointParams = {};
                pointParams.forwards = goforward;
                chassis.moveToPoint(x, y, 1000, pointParams);
                break;
            }
            case 't': {
                if (tokens.size() < 3) {
                    printf("Error: malformed turn command: %s\n", line.c_str());
                    break;
                }
                double theta = std::stod(tokens[2]);
                printf("Turn: theta=%f\n", theta);
                chassis.turnToHeading(theta, 1000);
                break;
            }
            case 'f': {
                if (tokens.size() < 3) {
                    printf("Error: malformed forward command: %s\n", line.c_str());
                    break;
                }
                double distance = std::stod(tokens[2]);
                printf("Drive Forward: distance=%f\n", distance);
                driveForwardToTarget(distance);
                break;
            }
            case 's': {
                if (tokens.size() < 3) {
                    printf("Error: malformed spin command: %s\n", line.c_str());
                    break;
                }
                double speed = std::stod(tokens[2]);
                bool reverse = (tokens[3] == "1");
                printf("Spin (%s): speed=%f Reversed:%d\n", device.c_str(), speed, reverse);
                if (reverse == 0) {
                    LOG_INFO("Spining a motor forward");
                    if (device == "a") Intake1.move_velocity(speed);
                    else if (device == "b") Intake2.move_velocity(speed);
                    else if (device == "c") Intake3.move_velocity(speed);
                    else printf("Unknown motor device: %s\n", device.c_str());
                } else if (reverse == 1) {
                    LOG_INFO("Spining a motor reverse");
                    if (device == "a") Intake1.move_velocity(-speed);
                    else if (device == "b") Intake2.move_velocity(-speed);
                    else if (device == "c") Intake3.move_velocity(-speed);
                    else printf("Unknown motor device: %s\n", device.c_str());
                }
                break;
            }
            case 'w': {
                if (tokens.size() < 3) {
                    printf("Error: malformed wait command: %s\n", line.c_str());
                    break;
                }
                double duration = std::stod(tokens[2]);
                printf("Wait: %f seconds\n", duration);
                pros::delay(duration * 1000); // PROS delay in ms
                break;
            }
            case 'p': {
                if (tokens.size() < 3) {
                    printf("Error: malformed piston command: %s\n", line.c_str());
                    break;
                }
                int value = std::stoi(tokens[2]);
                printf("Piston (%s): value=%d\n", device.c_str(), value);

                if (device == "d") descore.set_value(value);
                else if (device == "w") weedwacker.set_value(value);
                else if (device == "b") ballblock.set_value(value);
                else printf("Unknown piston device: %s\n", device.c_str());
                break;
            }
            default:
                printf("Unknown command type: %s\n", line.c_str());
                break;
        }
    }
    autonrunning = false; // Set the running flag to false after the auton is complete
}